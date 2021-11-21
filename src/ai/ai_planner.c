
#include "ai_planner.h"
#include "scene/level_scene.h"
#include "math/mathf.h"
#include "math/vector3.h"
#include "util/memory.h"

#define MINION_DISTANCE_SCALAR 0.25f

unsigned getClosestBaseFromPoint(struct LevelScene* levelScene, struct Vector3* from){
    unsigned minIdx = 0;
    float minDist = sqrtf(vector3DistSqrd(from, &levelScene->bases[0].position));
    for(unsigned i = 1; i < levelScene->baseCount; ++i){
        float dist = sqrtf(vector3DistSqrd(from, &levelScene->bases[i].position));
        if(dist < minDist){
            minIdx = i;
            minDist = dist;
        }
    }
    return minIdx;
}

float aiPlannerFindDistance(struct LevelScene* levelScene, struct Vector3* from, int toBase) {
    return getDistanceToBase(&levelScene->definition->pathfinding, 
            getClosestBaseFromPoint(levelScene, from), 
            toBase, 
            levelScene->baseCount);

    // return 0.0f;
    //return sqrtf(vector3DistSqrd(from, &levelScene->bases[toBase].position));
}

void aiPlannerScorePlan(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    switch (plan->planType) {
        case AIPlanTypeAttackBase:
            plan->estimatedCost = aiPlannerFindDistance(
                levelScene, 
                &levelScene->players[planner->teamNumber].transform.position, 
                plan->targetBase
            );
            break;
        case AIPlanTypeAttackBaseWithBase:
            plan->estimatedCost = aiPlannerFindDistance(
                levelScene, 
                &levelScene->players[planner->teamNumber].transform.position, 
                plan->param0
            );
            // score the minion distance partially
            plan->estimatedCost += aiPlannerFindDistance(
                levelScene, 
                &levelScene->bases[plan->param0].position, 
                plan->targetBase
            ) * MINION_DISTANCE_SCALAR;
            break;  
        default:
            plan->estimatedCost = 0.0f;
            break;
    }

    plan->estimatedCost *= 1 + plan->neededMinions;
}

int aiPlannerFindRandomBaseForTeam(struct LevelScene* levelScene, unsigned team, unsigned invertTeam) {
    unsigned count = 0;
    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        unsigned doesTeamMatch = levelBaseGetTeam(&levelScene->bases[i]) == team;
        
        if (doesTeamMatch != invertTeam) {
            ++count;
        }
    }

    unsigned randomIndex = randomInRange(0, count);
    count = 0;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        unsigned doesTeamMatch = levelBaseGetTeam(&levelScene->bases[i]) == team;
        
        if (doesTeamMatch != invertTeam) {
            if (count == randomIndex) {
                return i;
            }
            ++count;
        }
    }
    
    return -1;
}

int aiPlannerFindNearestBase(struct LevelScene* levelScene, unsigned fromBaseIndex, unsigned team, unsigned invertTeam) {
    // TODO
    int result = -1;
    float distance = 0.0f;

    struct Vector3* fromPos = &levelScene->bases[fromBaseIndex].position;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        float distSqrd = vector3DistSqrd(fromPos, &levelScene->bases[i].position);
        unsigned doesTeamMatch = levelBaseGetTeam(&levelScene->bases[i]) == team;

        if (doesTeamMatch != invertTeam && (result == -1 || distSqrd < distance)) {
            distance = distSqrd;
            result = i;
        }
    }

    return result;
}

int aiPlannerExtraMinionsNeededForCapture(struct LevelBase* base) {
    if (levelBaseGetTeam(base) == TEAM_NONE) {
        return 0;
    } else {
        return base->prevControlCount[base->team.teamNumber];
    }
}

void aiPlannerComeUpWithPlan(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    plan->planType = randomInRange(AIPlanTypeAttackBase, AIPlanCount);

    switch (plan->planType) {
        case AIPlanTypeAttackBase:
        {
            int targetBase = aiPlannerFindRandomBaseForTeam(levelScene, planner->teamNumber, 1);
            if (targetBase < 0) {
                plan->planType = AIPlanTypeNone;
                return;
            }
            plan->targetBase = targetBase;
            plan->neededMinions = aiPlannerExtraMinionsNeededForCapture(&levelScene->bases[targetBase]);
            break;
        }
        case AIPlanTypeAttackBaseWithBase:
        {
            int sourceBase = aiPlannerFindRandomBaseForTeam(levelScene, planner->teamNumber, 0);
            if (sourceBase < 0) {
                plan->planType = AIPlanTypeNone;
                return;
            }

            plan->param0 = sourceBase;
            int targetBase = aiPlannerFindNearestBase(levelScene, plan->param0, planner->teamNumber, 1);
            if (targetBase < 0) {
                plan->planType = AIPlanTypeNone;
                return;
            }
            plan->targetBase = targetBase;
            plan->neededMinions = aiPlannerExtraMinionsNeededForCapture(&levelScene->bases[targetBase]);
            break;
        }
        default:
            break;
    }
}

int aiPlannerIsPlanExecuted(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    switch (plan->planType) {
        case AIPlanTypeAttackBase:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) == planner->teamNumber; 
        case AIPlanTypeAttackBaseWithBase:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) == planner->teamNumber ||
                levelBaseGetTeam(&levelScene->bases[plan->param0]) != planner->teamNumber || 
                levelScene->bases[plan->param0].defaultComand == MinionCommandAttack;
        default:
            return 0;
    }
}

int aiPlannerIsPlanStillValid(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    switch (plan->planType) {
        case AIPlanTypeAttackBase:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) != planner->teamNumber; 
        case AIPlanTypeAttackBaseWithBase:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) != planner->teamNumber && 
                levelBaseGetTeam(&levelScene->bases[plan->param0]) == planner->teamNumber; 
        default:
            return 0;
    }
}

void aiPlannerImplementNextPlan(struct LevelScene* levelScene, struct AIPlanner* planner) {
    // exit early if there is no plan
    if (planner->nextPlan.planType == AIPlanTypeNone) {
        return;
    }

    struct AIPlan* nextPlan = 0;

    for (unsigned i = 0; i < MAX_ACTIVE_PLANS; ++i) {
        if (planner->activePlans[i].planType == AIPlanTypeNone) {
            nextPlan = &planner->activePlans[i];
            break;
        }
    }

    // exit if no activePlan slot is avaialable
    if (!nextPlan) {
        return;
    }

    *nextPlan = planner->nextPlan;
    planner->currentPlan = nextPlan;
    planner->thinkingTimer = MINIMUM_THIKING_FRAMES;

    if (nextPlan->targetBase < planner->baseCount) {
        planner->basesCoveredByPlan[nextPlan->targetBase] = nextPlan;
    }

    switch (nextPlan->planType) {
        case AIPlanTypeAttackBaseWithBase:
            planner->basesCoveredByPlan[nextPlan->param0] = nextPlan;
        default:
            break;
    }

    planner->nextPlan.planType = AIPlanTypeNone;
}

void aiPlannerEndPlan(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    plan->planType = AIPlanTypeNone;

    for (unsigned i = 0; i < planner->baseCount; ++i) {
        if (planner->basesCoveredByPlan[i] == plan) {
            planner->basesCoveredByPlan[i] = 0;
        }
    }

    if (planner->currentPlan == plan) {
        planner->currentPlan = 0;
    }
}

int aiPlannerDoesPlanConflict(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    switch (plan->planType) {
        case AIPlanTypeAttackBase:
            return planner->basesCoveredByPlan[plan->targetBase] != 0;
        case AIPlanTypeAttackBaseWithBase:
            return planner->basesCoveredByPlan[plan->targetBase] != 0 && planner->basesCoveredByPlan[plan->param0] != 0;
        default:
            return 0;
    }
}

void aiPlannerInit(struct AIPlanner* planner, unsigned teamNumber, unsigned baseCount) {
    zeroMemory(planner, sizeof(struct AIPlanner));
    planner->thinkingTimer = MINIMUM_THIKING_FRAMES;
    planner->teamNumber = teamNumber;
    planner->baseCount = baseCount;
    planner->basesCoveredByPlan = malloc(sizeof(struct AIPlan*) * baseCount);
    zeroMemory(planner->basesCoveredByPlan, sizeof(struct AIPlan*) * baseCount);
}

void aiPlannerUpdate(struct LevelScene* levelScene, struct AIPlanner* planner) {
    struct AIPlan newPlan;
    aiPlannerComeUpWithPlan(levelScene, planner, &newPlan);
    if (!aiPlannerDoesPlanConflict(levelScene, planner, &newPlan)) {
        // make sure next plan is up to date
        aiPlannerScorePlan(levelScene, planner, &planner->nextPlan);
        aiPlannerScorePlan(levelScene, planner, &newPlan);
        
        if (planner->nextPlan.planType == AIPlanTypeNone || (newPlan.planType != AIPlanTypeNone && planner->nextPlan.estimatedCost > newPlan.estimatedCost)) {
            planner->nextPlan = newPlan;
        }
    }

    // prune plans that are no longer valid
    for (unsigned i = 0; i < MAX_ACTIVE_PLANS; ++i) {
        if (planner->activePlans[i].planType != AIPlanTypeNone && !aiPlannerIsPlanStillValid(levelScene, planner, &planner->activePlans[i])) {
            aiPlannerEndPlan(levelScene, planner, &planner->activePlans[i]);
        }
    }

    if (planner->currentPlan && aiPlannerIsPlanExecuted(levelScene, planner, planner->currentPlan)) {
        planner->currentPlan = 0;
        planner->thinkingTimer = MINIMUM_THIKING_FRAMES;
    } else if (!planner->currentPlan && planner->thinkingTimer == 0) {
        aiPlannerImplementNextPlan(levelScene, planner);
    } else if (planner->thinkingTimer > 0) {
        // thinking timer is used to ensure the AI comes up with a few 
        // plans before trying to execute the best option
        --planner->thinkingTimer;
    }
}

struct LevelBase* aiPlannerGetTargetBase(struct LevelScene* levelScene, struct AIPlanner* planner) {
    if (!planner->currentPlan) {
        return 0;
    }

    switch (planner->currentPlan->planType) {
        case AIPlanTypeAttackBase:
            return &levelScene->bases[planner->currentPlan->targetBase];
        case AIPlanTypeAttackBaseWithBase:
            return &levelScene->bases[planner->currentPlan->param0];
        default:
            return 0;
    }
}

struct Vector3* aiPlannerGetTarget(struct LevelScene* levelScene, struct AIPlanner* planner) {
    struct LevelBase* base = aiPlannerGetTargetBase(levelScene, planner);

    if (base) {
        return &base->position;
    } else {
        return 0;
    }
}