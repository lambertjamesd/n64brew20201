#include "ai_planner.h"
#include "scene/level_scene.h"
#include "math/mathf.h"

float aiPlannerFindDistance(struct LevelScene* levelScene, struct Vector3* from, int toBase) {
    // TODO
    return 0.0f;
}

void aiPlannerScorePlan(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    // TODO
    plan->estimatedCost = 0.0f;
}

int aiPlannerFindRandomBaseForTeam(struct LevelScene* levelScene, unsigned team, unsigned invertTeam) {
    unsigned count = 0;
    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        unsigned doesTeamMatch = levelBaseGetTeam(&levelScene->bases[i]) == team;
        
        if (doesTeamMatch != invertTeam) {
            ++count;
        }
    }

    count = 0;
    unsigned randomIndex = randomInRange(0, count);

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
    return -1;
}

int aiPlannerMinionsNeededForCapture(struct LevelBase* base) {
    if (levelBaseGetTeam(base) == TEAM_NONE) {
        return 1;
    } else {
        return base->prevControlCount[base->team.teamNumber] + 1;
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
            plan->neededMinions = aiPlannerMinionsNeededForCapture(&levelScene->bases[targetBase]) - 1;
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
            plan->neededMinions = aiPlannerMinionsNeededForCapture(&levelScene->bases[targetBase]);
            break;
        }
        default:
            break;
    }
}

int aiPlannerIsPlanExecuted(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    // TODO
    return 0;
}

int aiPlannerIsPlanStillValid(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    // TODO
    return 0;
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

    if (nextPlan->targetBase < planner->baseCount) {
        planner->basesCoveredByPlan[nextPlan->targetBase] = nextPlan;
    }
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

    if (!planner->currentPlan) {
        aiPlannerImplementNextPlan(levelScene, planner);
    }
}

void aiPlannerUpdate(struct LevelScene* levelScene, struct AIPlanner* planner) {
    struct AIPlan newPlan;
    aiPlannerComeUpWithPlan(levelScene, planner, &newPlan);
    // make sure next plan is up to date
    aiPlannerScorePlan(levelScene, planner, &planner->nextPlan);
    
    if (planner->nextPlan.planType == AIPlanTypeNone || planner->nextPlan.estimatedCost > newPlan.estimatedCost) {
        planner->nextPlan = newPlan;
    }

    // prune plans that are no longer valid
    for (unsigned i = 0; i < MAX_ACTIVE_PLANS; ++i) {
        if (planner->activePlans[i].planType != AIPlanTypeNone && !aiPlannerIsPlanStillValid(levelScene, planner, &planner->activePlans[i])) {
            aiPlannerEndPlan(levelScene, planner, &planner->activePlans[i]);
        }
    }

    if (aiPlannerIsPlanExecuted(levelScene, planner, planner->currentPlan)) {
        // if the current plan surivived pruning in the last step but is finished then
        // set it to null so another plan can be started
        planner->currentPlan = 0;
        aiPlannerImplementNextPlan(levelScene, planner);
    }
}