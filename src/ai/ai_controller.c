#include "ai_controller.h"
#include "collision/collisionlayers.h"
#include "scene/scene_management.h"
#include "util/time.h"
#include "math/mathf.h"

#define ATTACK_RADIUS (10.0f * SCENE_SCALE)

#define MOVEMENT_SLOWDOWN_RADIUS    (1.0f * SCENE_SCALE)

#define PUNCH_DELAY_EASY 3.0f
#define PUNCH_DELAY_HARD 0.0f

#define SPEED_SCALAR_EASY 0.7f

struct CollisionCircle gBotCollider = {
    {CollisionShapeTypeCircle},
    ATTACK_RADIUS,
};

void ai_TriggerVision(struct DynamicSceneOverlap* overlap) {
    if (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) {
        struct AIController* bot = (struct AIController*)overlap->thisEntry->data;
        struct Player* player = (struct Player*)&gCurrentLevel.players[bot->playerIndex];
        struct TeamEntity* entityB = (struct TeamEntity*)overlap->otherEntry->data;

        if (entityB->teamNumber != player->team.teamNumber && 
            (!bot->attackTarget || aiAttackPriority(bot->attackTarget) < aiAttackPriority(entityB))) {
            bot->attackTarget = entityB;
        }
    }
}

unsigned getNumNeutralBases(struct LevelBase* bases, unsigned numBases){
    unsigned count = 0;
    for(unsigned i = 0; i < numBases; ++i) if(bases[i].team.teamNumber == TEAM_NONE) count++;
    return count;
}

void ai_Init(struct AIController* inController, struct PathfindingDefinition* pathfinder, unsigned playerIndex, unsigned teamIndex, unsigned baseCount, float difficulty) {
    inController->pathfindingInfo = pathfinder;
    inController->playerIndex = playerIndex;
    inController->teamIndex = teamIndex;
    inController->botAction = 0;
    inController->currTarget = gZeroVec;
    inController->attackTarget = 0;
    inController->numMinions = 0;
    inController->punchDelay = 0;
    inController->difficulty = difficulty;
    aiPlannerInit(&inController->planner, teamIndex, baseCount, difficulty);
    pathfinderInit(&inController->pathfinder, &gZeroVec);

    struct Vector2 position;

    position.x = gCurrentLevel.players[playerIndex].transform.position.x;
    position.y = gCurrentLevel.players[playerIndex].transform.position.z;

    inController->collider = dynamicSceneNewEntry(
        &gBotCollider.shapeCommon, 
        inController, 
        &position,
        ai_TriggerVision,
        DynamicSceneEntryIsTrigger,
        CollisionLayersAllTeams ^ COLLISION_LAYER_FOR_TEAM(teamIndex)
    );
}

void ai_update(struct LevelScene* level, struct AIController* ai) {
    struct Player* player = &level->players[ai->playerIndex];

    if (!playerIsAlive(player)) {
        aiPlannerReset(&ai->planner);
        return;
    }

    dynamicEntrySetPos3D(ai->collider, &gCurrentLevel.players[ai->playerIndex].transform.position);

    aiPlannerUpdate(level, &ai->planner);
    pathfinderUpdate(&ai->pathfinder, &level->definition->pathfinding, &level->players[ai->playerIndex].transform.position, &level->players[ai->playerIndex].transform.position);

    if (ai->planner.currentPlan) {
        switch (ai->planner.currentPlan->planType) {
            case AIPlanTypeAttackBaseWithBase:
                if (gPlayerAtBase[ai->playerIndex] && 
                    gPlayerAtBase[ai->playerIndex]->baseId == ai->planner.currentPlan->targetBase) {
                    levelBaseSetDefaultCommand(gPlayerAtBase[ai->playerIndex], MinionCommandAttack, ai->playerIndex);
                }
                break;
            case AIPlanTypeUpgrade:
                if (gPlayerAtBase[ai->playerIndex] && 
                    gPlayerAtBase[ai->playerIndex]->baseId == ai->planner.currentPlan->targetBase) {
                    levelBaseStartUpgrade(gPlayerAtBase[ai->playerIndex], ai->planner.currentPlan->param0);
                }
                break;
            default:
                break;
        }

        if (ai->planner.currentPlan->targetBase < level->baseCount) {
            unsigned baseNode = getBasePathNodeID(&level->definition->pathfinding, ai->planner.currentPlan->targetBase);

            if (baseNode == NODE_NONE) {
                pathfinderReset(&ai->pathfinder);
            } else if (baseNode != ai->pathfinder.targetNode) {
                pathfinderSetTarget(
                    &ai->pathfinder, 
                    &level->definition->pathfinding, 
                    &player->transform.position, 
                    aiPlannerGetTarget(level, &ai->planner)
                );
            }
        }
    } else {
        pathfinderReset(&ai->pathfinder);
    }

    if (ai->attackTarget && (
        vector3DistSqrd(&player->transform.position, teamEntityGetPosition(ai->attackTarget)) > ATTACK_RADIUS * ATTACK_RADIUS || 
        !teamEntityIsAlive(ai->attackTarget))
    ) {
        ai->attackTarget = 0;
    }
    
    if (ai->punchDelay > 0.0f) {
        ai->punchDelay -= gTimeDelta;
    }
}

unsigned isTargetBaseCaptured(struct AIController* ai){
    unsigned baseIdx = getBaseFromNodeId(&gCurrentLevel.definition->pathfinding, ai->pathfinder.targetNode, gCurrentLevel.baseCount);
    if(baseIdx != -1){
        return gCurrentLevel.bases[baseIdx].team.teamNumber == ai->teamIndex;
    }
    else return 0;
}

void ai_collectPlayerInput(struct LevelScene* levelScene, struct AIController* ai, struct PlayerInput* playerInput) {
    struct Player* player = &levelScene->players[ai->playerIndex];
    struct Vector3* targetPosition = 0;

    playerInput->actionFlags = 0;

    struct TeamEntity* attackTarget = ai->attackTarget;

    if (player->aiTarget) {
        attackTarget = player->aiTarget;
    }

    if(attackTarget != 0){
        targetPosition = teamEntityGetPosition(attackTarget);

        float attackRadius = attackTarget->entityType == TeamEntityTypePlayer ? PLAYER_COLLIDER_RADIOUS * 2.0f : (PLAYER_COLLIDER_RADIOUS + MINION_COLLIDE_RADIUS);
        attackRadius += 0.1f * SCENE_SCALE;

        if (player->aiTarget != 0 || vector3DistSqrd(&player->transform.position, targetPosition) < attackRadius * attackRadius) {
            if (!(playerInput->prevActions & PlayerInputActionsAttack) && ai->punchDelay <= 0.0f) {
                playerInput->actionFlags |= PlayerInputActionsAttack;
                ai->punchDelay = mathfLerp(PUNCH_DELAY_EASY, PUNCH_DELAY_HARD, ai->difficulty);
            }
        }
    } else if(ai->planner.currentPlan && ai->pathfinder.currentNode < levelScene->definition->pathfinding.nodeCount) {
        targetPosition = &levelScene->definition->pathfinding.nodePositions[ai->pathfinder.currentNode];
    }

    if (targetPosition) {
        vector3Sub(targetPosition, &player->transform.position, &playerInput->targetWorldDirection);

        float magnitude = vector3MagSqrd(&playerInput->targetWorldDirection);

        if (magnitude < MOVEMENT_SLOWDOWN_RADIUS * MOVEMENT_SLOWDOWN_RADIUS) {
            vector3Scale(&playerInput->targetWorldDirection, &playerInput->targetWorldDirection, 1.0f / MOVEMENT_SLOWDOWN_RADIUS);
        } else {        
            vector3Normalize(&playerInput->targetWorldDirection, &playerInput->targetWorldDirection);
        }
    } else {
        playerInputNoInput(playerInput);
        playerInput->targetWorldDirection = gZeroVec;
    }

    vector3Scale(&playerInput->targetWorldDirection, &playerInput->targetWorldDirection, mathfLerp(SPEED_SCALAR_EASY, 1.0f, ai->difficulty));
}