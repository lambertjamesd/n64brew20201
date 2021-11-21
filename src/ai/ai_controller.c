#include "ai_controller.h"
#include "scene/scene_management.h"



unsigned getNumNeutralBases(struct LevelBase* bases, unsigned numBases){
    unsigned count = 0;
    for(unsigned i = 0; i < numBases; ++i) if(bases[i].team.teamNumber == TEAM_NONE) count++;
    return count;
}

void ai_getClosestEnemyCharacter(const unsigned playerIndex, struct TeamEntity* outEnemy, float* dist){
    unsigned team = gCurrentLevel.players[playerIndex].team.teamNumber;
    unsigned minIdx = -1;
    float minDist = ~0;
    float currDist = ~0;
    struct TeamEntity* outEntity = NULL;

    for(unsigned i = 0; i < gCurrentLevel.playerCount + gCurrentLevel.minionCount; ++i){
        if(i < gCurrentLevel.playerCount){
            if(gCurrentLevel.players[i].team.teamNumber != team){
                currDist = vector3DistSqrd(
                    &gCurrentLevel.players[playerIndex].transform.position, 
                    &gCurrentLevel.players[i].transform.position);
                if(minIdx == -1 || currDist < minDist){
                    minIdx = i;
                    minDist = currDist;
                }
            }
        }
        else{
            if(gCurrentLevel.minions[i - gCurrentLevel.playerCount].team.teamNumber != team){
                currDist = vector3DistSqrd(
                    &gCurrentLevel.players[playerIndex].transform.position, 
                    &gCurrentLevel.minions[i - gCurrentLevel.playerCount].transform.position);
                if(minIdx == -1 || currDist < minDist){
                    minIdx = i;
                    minDist = currDist;
                }
            }
        }
    }
    if(minIdx != -1){
        if(minIdx < gCurrentLevel.playerCount){
            outEntity = (struct TeamEntity*)&gCurrentLevel.players[minIdx];
        }
        else outEntity = (struct TeamEntity*)&gCurrentLevel.minions[minIdx-gCurrentLevel.playerCount];
    }
    dist = &minDist;
    outEnemy = outEntity;
}

void ai_Init(struct AIController* inController, struct PathfindingDefinition* pathfinder, unsigned playerIndex, unsigned teamIndex, unsigned baseCount){
    inController->pathfindingInfo = pathfinder;
    inController->playerIndex = playerIndex;
    inController->teamIndex = teamIndex;
    inController->botAction = 0;
    inController->currTarget = gZeroVec;
    inController->attackTarget = 0;
    inController->numMinions = 0;
    aiPlannerInit(&inController->planner, teamIndex, baseCount);
    pathfinderInit(&inController->pathfinder, &gZeroVec);
}

void ai_update(struct LevelScene* level, struct AIController* ai) {
    aiPlannerUpdate(level, &ai->planner);
    pathfinderUpdate(&ai->pathfinder, &level->definition->pathfinding, &level->players[ai->playerIndex].transform.position, &level->players[ai->playerIndex].transform.position);

    if (ai->planner.currentPlan) {
        switch (ai->planner.currentPlan->planType) {
            case AIPlanTypeAttackBaseWithBase:
                if (gPlayerAtBase[ai->playerIndex] && 
                    gPlayerAtBase[ai->playerIndex]->baseId == ai->planner.currentPlan->param0) {
                    levelBaseSetDefaultCommand(gPlayerAtBase[ai->playerIndex], MinionCommandAttack, ai->playerIndex);
                }
                break;
            default:
                break;
        }
    }
}

unsigned short wasPlayerJustHit(struct AIController* ai){
    return (ai && !ai->attackTarget && gCurrentLevel.players[ai->playerIndex].damageHandler.damageTimer > 0.f);
}

void ai_collectPlayerInput(struct LevelScene* levelScene, struct AIController* ai, struct PlayerInput* playerInput) {
    struct Vector3* targetPosition;// = aiPlannerGetTarget(levelScene, &ai->planner);
    struct Vector3* enemyPosition;
    float enemyDist = -1.f;

    if(!ai->attackTarget && wasPlayerJustHit(ai))
        ai_getClosestEnemyCharacter(ai->playerIndex, ai->attackTarget, &enemyDist);
    if(ai->attackTarget){
        if(enemyDist > -1.f) enemyDist = vector3DistSqrd(&gCurrentLevel.players[ai->playerIndex].transform.position,
        enemyPosition = teamEntityGetPosition(ai->attackTarget));
        if(enemyDist > 2000 || !teamEntityIsAlive(ai->attackTarget)) ai->attackTarget = NULL;
    }

    if(ai->attackTarget) targetPosition = enemyPosition;
    else if(ai->planner.currentPlan && 
        ai->planner.currentPlan->targetBase >= 0 && (ai->planner.currentPlan->targetBase < gCurrentLevel.baseCount)){
        if(ai->pathfinder.currentNode < levelScene->definition->pathfinding.nodeCount &&
            (levelScene->definition->pathfinding.baseNodes[ai->planner.currentPlan->targetBase] == 
            ai->pathfinder.targetNode)){

                //unsigned nodeIdx = gCurrentLevel.definition->pathfinding.baseNodes[ai->planner.currentPlan->targetBase];
                targetPosition = &gCurrentLevel.definition->pathfinding.nodePositions[ai->pathfinder.currentNode];
            
        }
        else {
            pathfinderSetTarget(&ai->pathfinder, 
            &levelScene->definition->pathfinding, 
            &levelScene->players[ai->playerIndex].transform.position, 
            &levelScene->bases[ai->planner.currentPlan->targetBase].position);

            //unsigned nodeIdx = gCurrentLevel.definition->pathfinding.baseNodes[ai->planner.currentPlan->targetBase];
            targetPosition = &gCurrentLevel.definition->pathfinding.nodePositions[ai->pathfinder.currentNode];
        }
    }

    playerInput->actionFlags = 0;

    if (targetPosition) {
        vector3Sub(targetPosition, &levelScene->players[ai->playerIndex].transform.position, &playerInput->targetWorldDirection);
        vector3Normalize(&playerInput->targetWorldDirection, &playerInput->targetWorldDirection);
    } else {
        playerInputNoInput(playerInput);
        playerInput->targetWorldDirection = gZeroVec;
        gCurrentLevel.players[ai->playerIndex].velocity = gZeroVec;
    }
}

struct LevelBase* ai_getClosestUncapturedBase(struct AIController* inController, struct LevelBase* bases, unsigned baseCount, struct Vector3* closeTo, unsigned team, unsigned short usePathfinding){
    unsigned int minIndex = 0;
    //if(usePathfinding == 1){
    //    minIndex = getClosestNeutralBase(inController->pathfindingInfo, bases, baseCount, inController->targetBase->baseId);
    //    if(minIndex == -1) minIndex = getClosestEnemyBase(inController->pathfindingInfo, bases, baseCount, inController->targetBase->baseId, team);
    //}
    //else{ //in the beginning of the game we still might want to compare the base position to player's location since there will be no targetBase reference
        struct Vector3 basePos;
        basePos.x = bases[0].position.x;
        basePos.y = bases[0].position.y;
        basePos.z = bases[0].position.z;
        float minDist = vector3DistSqrd(&basePos, closeTo);
        for(unsigned int i = 1; i < baseCount; i++){
            if(levelBaseGetTeam(&bases[i]) == team) continue;
            basePos.x = bases[i].position.x;
            basePos.y = bases[i].position.y;
            basePos.z = bases[i].position.z;
            float currDist = vector3DistSqrd(&basePos, closeTo);
            if(currDist < minDist){
                minIndex = i;
                minDist = currDist;
            }
        }
    //}
    struct LevelBase* outBase = &bases[minIndex];
    return outBase;
}
