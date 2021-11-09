#include "ai_controller.h"

void InitAI(struct AIController* inController, unsigned playerIndex, unsigned teamIndex){
    inController->playerIndex = playerIndex;
    inController->teamIndex = teamIndex;
    inController->botAction = 0;
    inController->currTarget = gZeroVec;
    inController->targetBase = 0;
    inController->attackTarget = 0;
}

void moveAItowardsTarget(struct AIController* inController, struct Vector3* currLocation, struct PlayerInput* inputRef){
    inputRef->actionFlags = 0;
    inController->botAction = (inController->attackTarget != 0 && 
    teamEntityIsAlive(inController->attackTarget)) ? 1 : 0;

    if(inController->botAction == 1){
        struct Vector3* newTarget = teamEntityGetPosition(inController->attackTarget);
        inController->currTarget.x = newTarget->x;
        inController->currTarget.y = newTarget->y;
        inController->currTarget.z = newTarget->z;

        unsigned int distToTarget = vector3DistSqrd(currLocation, &inController->currTarget);
        
       if(&inController->targetBase != NULL && vector3DistSqrd(&inController->targetBase->position, &inController->currTarget) > 825){
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
    else if(inController->targetBase != NULL) {
        inController->currTarget = inController->targetBase->position;
        unsigned baseDistance = vector3DistSqrd(currLocation, &inController->currTarget);
        if (baseDistance > 625)
            inController->botAction = 2;
        else {
            inController->botAction = 0;
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

void setTargetBase(struct AIController* inController, struct LevelBase* inBase){
    inController->targetBase = inBase;
}

struct LevelBase* getClosestUncapturedBase(struct LevelBase* bases, unsigned baseCount, struct Vector3* closeTo, unsigned team){
    struct Vector3 basePos;
    basePos.x = bases[0].position.x;
    basePos.y = bases[0].position.y;
    basePos.z = bases[0].position.z;
    
    float minDist = vector3DistSqrd(&basePos, closeTo);
    unsigned int minIndex = 0;
    for(unsigned int i = 1; i < baseCount; i++){
        if(levelBaseGetFactionID(&bases[i]) == team) continue;
        basePos.x = bases[i].position.x;
        basePos.y = bases[i].position.y;
        basePos.z = bases[i].position.z;
        float currDist = vector3DistSqrd(&basePos, closeTo);
        if(currDist < minDist){
            minIndex = i;
            minDist = currDist;
        }
    }
    struct LevelBase* outBase = &bases[minIndex];
    return outBase;
}
