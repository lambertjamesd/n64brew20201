#include "ai_controller.h"
#include "ai_pathfinder.h"
#include "scene/scene_management.h"

unsigned getNumNeutralBases(struct LevelBase* bases, unsigned numBases){
    unsigned count = 0;
    for(unsigned i = 0; i < numBases; ++i) if(bases[i].team.teamNumber == TEAM_NONE) count++;
    return count;
}

void ai_Init(struct AIController* inController, struct PathfindingDefinition* pathfinder, unsigned playerIndex, unsigned teamIndex, unsigned baseCount){
    inController->pathfindingInfo = pathfinder;
    inController->playerIndex = playerIndex;
    inController->teamIndex = teamIndex;
    inController->botAction = 0;
    inController->currTarget = gZeroVec;
    inController->attackTarget = 0;
    inController->isUsingPathfinding = 0;
    inController->numMinions = 0;
    aiPlannerInit(&inController->planner, teamIndex, baseCount);
}

void ai_moveTowardsTarget(struct AIController* inController, struct Vector3* currLocation, struct PlayerInput* inputRef){
    inputRef->actionFlags = 0;
    inController->botAction = (inController->attackTarget != 0 && 
    teamEntityIsAlive(inController->attackTarget)) ? 1 : 0;

    struct LevelBase* targetBase = aiPlannerGetTargetBase(&gCurrentLevel, &inController->planner);

    if(inController->botAction == 1){
        struct Vector3* newTarget = teamEntityGetPosition(inController->attackTarget);
        inController->currTarget.x = newTarget->x;
        inController->currTarget.y = newTarget->y;
        inController->currTarget.z = newTarget->z;

        unsigned int distToTarget = vector3DistSqrd(currLocation, &inController->currTarget);
        
       if(targetBase != NULL && vector3DistSqrd(&targetBase->position, &inController->currTarget) > 825){
            inController->attackTarget = 0;
            inController->botAction = 0;
        }

        if(distToTarget <= 800){ 
            inputRef->actionFlags = PlayerInputActionsAttack;
            if(distToTarget <= 625){
                inController->botAction = 0;
            }
        }
    }
    else if(targetBase != NULL) {
        if(inController->isUsingPathfinding == 1){
            unsigned navTargetDistance = vector3DistSqrd(currLocation, &inController->currTarget);
            if(navTargetDistance <= 200){
                unsigned indFrom = inController->lastPathfidningIndex;
                unsigned indTo = nav_getClosestPoint(&targetBase->position, inController->pathfindingInfo->nodePositions, inController->pathfindingInfo->nodeCount);
               inController->lastPathfidningIndex = nav_getNextNavPoint(indFrom, indTo, inController->pathfindingInfo->nextNode, inController->pathfindingInfo->nodeCount);
               inController->currTarget = inController->pathfindingInfo->nodePositions[inController->lastPathfidningIndex];
            }
        }
        else inController->currTarget = targetBase->position;
        
        unsigned baseDistance = vector3DistSqrd(currLocation, &targetBase->position);
        if (baseDistance > 625)
            inController->botAction = 2;
        else {
            inController->botAction = 0;
            inController->isUsingPathfinding = 0;
        }
    }

    if(inController->botAction == 1 || inController->botAction == 2){
        struct Vector3 dir;

        vector3Sub(&inController->currTarget, currLocation, &dir);
        vector3Normalize(&dir, &dir);
        inputRef->targetWorldDirection = dir;
    }
    else{
        inputRef->targetWorldDirection.x = 0;
        inputRef->targetWorldDirection.y = 0;
        inputRef->targetWorldDirection.z = 0;

    }
}

void ai_update(struct LevelScene* level, struct AIController* ai) {
    aiPlannerUpdate(level, &ai->planner);

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

void ai_collectPlayerInput(struct LevelScene* levelScene, struct AIController* ai, struct PlayerInput* playerInput) {
    struct Vector3* targetPosition = aiPlannerGetTarget(levelScene, &ai->planner);

    playerInput->actionFlags = 0;

    if (targetPosition) {
        vector3Sub(targetPosition, &levelScene->players[ai->playerIndex].transform.position, &playerInput->targetWorldDirection);
        vector3Normalize(&playerInput->targetWorldDirection, &playerInput->targetWorldDirection);
    } else {
        playerInput->targetWorldDirection = gZeroVec;
    }

    // //if the player just got hit 
    // if(ai->attackTarget == NULL && levelScene->players[ai->playerIndex].damageHandler.damageTimer > 0.0f){
    //     float minionDist;
    //     float playerDist;

    //     struct Player* playerEnt = levelGetClosestEnemyPlayer(
    //         levelScene,
    //         &levelScene->players[ai->playerIndex].transform.position,
    //         ai->playerIndex,
    //         &playerDist
    //     );

    //     struct Minion* minionEnt = levelGetClosestEnemyMinion(
    //         levelScene,
    //         &levelScene->players[ai->playerIndex].transform.position,
    //         ai->playerIndex,
    //         &minionDist
    //     );

    //     if(minionEnt && minionDist < playerDist) {
    //         if(minionEnt->team.teamNumber != levelScene->players[ai->playerIndex].team.teamNumber) {
    //             ai->attackTarget = (struct TeamEntity*)minionEnt;
    //         }
    //     } else {
    //         if(playerEnt->team.teamNumber != levelScene->players[ai->playerIndex].team.teamNumber) {
    //             ai->attackTarget = (struct TeamEntity*)playerEnt;
    //         }
    //     }
    // }

    // ai_moveTowardsTarget(
    //     ai, 
    //     &levelScene->players[ai->playerIndex].transform.position, 
    //     playerInput
    // );

    // if(playerInput->targetWorldDirection.x == 0 && playerInput->targetWorldDirection.y == 0 && playerInput->targetWorldDirection.z == 0){
    //     playerInputNoInput(playerInput);
    //     levelScene->players[ai->playerIndex].velocity.x = 0;
    //     levelScene->players[ai->playerIndex].velocity.y = 0;
    //     levelScene->players[ai->playerIndex].velocity.z = 0;
    // } 
}

struct LevelBase* ai_getClosestUncapturedBase(struct AIController* inController, struct LevelBase* bases, unsigned baseCount, struct Vector3* closeTo, unsigned team, unsigned short usePathfinding){
    unsigned int minIndex = 0;
    if(usePathfinding == 1){
        // minIndex = getClosestNeutralBase(inController->pathfindingInfo, bases, baseCount, inController->targetBase->baseId);
        // if(minIndex == -1) minIndex = getClosestEnemyBase(inController->pathfindingInfo, bases, baseCount, inController->targetBase->baseId, team);
    }
    else{ //in the beginning of the game we still might want to compare the base position to player's location since there will be no targetBase reference
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
    }
    struct LevelBase* outBase = &bases[minIndex];
    return outBase;
}
