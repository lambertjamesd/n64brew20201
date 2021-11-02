#ifndef _SCENE_AI_H
#define _SCENE_AI_H

#include "math/vector3.h"
#include "levelbase.h"
#include "playerinput.h"
#include "teamentity.h"


#define MOVETO_ACCEPTANCE_RADIUS  10
#define BASE_ACCEPTANCE_RADIUS 10
#define ATTACK_ACCEPTANCE_RADIUS 10

struct AIController{
    unsigned botAction; //0 - nothing, 1 - attack, 2 - go to the base
    unsigned playerIndex;
    unsigned teamIndex;

    struct Vector3 currTarget;
    struct LevelBase* targetBase;
    struct TeamEntity* attackTarget;
};

void moveAItowardsTarget(struct AIController* inController, struct Vector3* currLocation, struct PlayerInput* inputRef);
void InitAI(struct AIController* inController, unsigned playerIndex, unsigned teamIndex);
void setTargetBase(struct AIController* inController, struct LevelBase* inBase);
struct LevelBase* getClosestUncapturedBase(struct LevelBase* bases, unsigned baseCount, struct Vector3* closeTo, unsigned team);


#endif