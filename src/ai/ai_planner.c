
#include "ai_planner.h"
#include "scene/level_scene.h"
#include "math/mathf.h"
#include "math/vector3.h"
#include "util/memory.h"
#include "scene/scene_management.h"
#include "util/time.h"

#define MINION_DISTANCE_SCALAR 0.25f
#define DISTANCE_SCALAR        (1.0f / (PLAYER_BASE_MOVE_SPEED * SCENE_SCALE))

#define UPGRADE_EASY_SCALAR     2.0f

#define EASY_THINKING_TIME      5.0f
#define HARD_THINKING_TIME      0.75f

#define HARD_PLANS_PER_FRAME    12
#define EASY_PLANS_PER_FRAME    1

float aiPlannerThinkingTime(struct AIPlanner* planner) {
    return mathfLerp(EASY_THINKING_TIME, HARD_THINKING_TIME, planner->difficulty);
}

int aiPlannerDoesTeamMatch(unsigned fromTeam, unsigned toTeam, enum TeamBaseType teamType) {
    switch (teamType)
    {
    case CurrentTeam:
        return fromTeam == toTeam;
    case EnemyTeam:
        return fromTeam != toTeam;
    case AnyTeam:
        return 1;
    }

    return 0;
}

unsigned getClosestBaseFromPoint(struct LevelScene* levelScene, struct Vector3* from){
    unsigned minIdx = 0;
    float minDist = vector3DistSqrd(from, &levelScene->bases[0].position);
    for(unsigned i = 1; i < levelScene->baseCount; ++i){
        float dist = vector3DistSqrd(from, &levelScene->bases[i].position);
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
}

float gUpgradeValueScalars[] = {
    0.4f,
    0.2f,
    0.8f,
};

void aiPlannerScorePlan(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    switch (plan->planType) {
        case AIPlanTypeAttackBase:
            plan->estimatedCost = aiPlannerFindDistance(
                levelScene, 
                &levelScene->players[planner->teamNumber].transform.position, 
                plan->targetBase
            ) * DISTANCE_SCALAR * planner->baseMultiplyer[plan->targetBase];
            break;
        case AIPlanTypeAttackBaseWithBase:
            plan->estimatedCost = aiPlannerFindDistance(
                levelScene, 
                &levelScene->players[planner->teamNumber].transform.position, 
                plan->targetBase
            ) * DISTANCE_SCALAR;
            // score the minion distance partially
            plan->estimatedCost += aiPlannerFindDistance(
                levelScene, 
                &levelScene->bases[plan->targetBase].position, 
                plan->param0
            ) * MINION_DISTANCE_SCALAR * DISTANCE_SCALAR;
            break;  
        case AIPlanTypeUpgrade:
        {
            float timeToUpgrade = levelBaseTimeForUpgrade(&levelScene->bases[plan->targetBase], plan->param0);

            if (timeToUpgrade < 0.0f) {
                plan->estimatedCost = 1000000000000000.0f;
            } else {
                plan->estimatedCost = aiPlannerFindDistance(
                    levelScene, 
                    &levelScene->players[planner->teamNumber].transform.position, 
                    plan->targetBase
                ) * DISTANCE_SCALAR + timeToUpgrade * gUpgradeValueScalars[plan->param0 - LevelBaseStateUpgradingSpawnRate];
            }

            plan->estimatedCost *= mathfLerp(UPGRADE_EASY_SCALAR, 1.0f, planner->difficulty);

            break;
        }
        default:
            plan->estimatedCost = 0.0f;
            break;
    }

    plan->estimatedCost *= 1 + plan->neededMinions;
}

int aiPlannerFindRandomBaseForTeam(struct LevelScene* levelScene, unsigned team, enum TeamBaseType teamType) {
    unsigned count = 0;
    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        unsigned doesTeamMatch = levelBaseGetTeam(&levelScene->bases[i]) == team;
        
        if (doesTeamMatch != teamType || teamType == AnyTeam) {
            ++count;
        }
    }

    unsigned randomIndex = randomInRange(0, count);
    count = 0;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {        
        if (aiPlannerDoesTeamMatch(levelBaseGetTeam(&levelScene->bases[i]), team, teamType)) {
            if (count == randomIndex) {
                return i;
            }
            ++count;
        }
    }
    
    return -1;
}

int aiPlannerFindNearestBase(struct LevelScene* levelScene, unsigned fromBaseIndex, unsigned team, enum TeamBaseType teamType) {
    int result = -1;
    float distance = 0.0f;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        if (i == fromBaseIndex) {
            continue;
        }

        float dist = getDistanceToBase(&levelScene->definition->pathfinding, fromBaseIndex, i, levelScene->baseCount);

        if (aiPlannerDoesTeamMatch(levelBaseGetTeam(&levelScene->bases[i]), team, teamType) && (result == -1 || dist < distance)) {
            distance = dist;
            result = i;
        }
    }

    return result;
}

int aiPlannerFindNearestBaseToPoint(struct LevelScene* levelScene, struct Vector3* from, unsigned team, enum TeamBaseType teamType, float* distanceOut) {
    unsigned clostestNode = nav_getClosestPoint(&levelScene->definition->pathfinding, from, distanceOut);

    int result = -1;
    float distance = 0.0f;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        unsigned baseNode = levelScene->definition->pathfinding.baseNodes[i];
        float dist = nav_getDistanceBetweenNodes(&levelScene->definition->pathfinding, baseNode, clostestNode);

        if (aiPlannerDoesTeamMatch(levelBaseGetTeam(&levelScene->bases[i]), team, teamType) && (result == -1 || dist < distance)) {
            distance = dist;
            result = i;
        }
    }

    if (distanceOut) {
        *distanceOut = sqrtf(*distanceOut) + distance;
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
            int moveToBase = aiPlannerFindRandomBaseForTeam(levelScene, planner->teamNumber, 0);
            if (moveToBase < 0) {
                plan->planType = AIPlanTypeNone;
                return;
            }

            plan->targetBase = moveToBase;
            int attackBase = aiPlannerFindNearestBase(levelScene, plan->targetBase, planner->teamNumber, 1);
            if (attackBase < 0) {
                plan->planType = AIPlanTypeNone;
                return;
            }
            plan->param0 = attackBase;
            plan->neededMinions = aiPlannerExtraMinionsNeededForCapture(&levelScene->bases[attackBase]);
            break;
        }
        case AIPlanTypeUpgrade:
        {
            if ((gCurrentLevel.levelFlags & LevelMetadataFlagsDisallowUpgrade) != 0) {
                plan->planType = AIPlanTypeNone;
                return;
            }

            int sourceBase = aiPlannerFindRandomBaseForTeam(levelScene, planner->teamNumber, 0);
            if (sourceBase < 0 || levelBaseIsBeingUpgraded(&levelScene->bases[sourceBase])) {
                plan->planType = AIPlanTypeNone;
                return;
            }

            plan->targetBase = sourceBase;
            plan->param0 = randomInRange(LevelBaseStateUpgradingSpawnRate, LevelBaseStateUpgradingDefence + 1);
            plan->neededMinions = 0;

            if (levelBaseIsBeingUpgraded(&levelScene->bases[sourceBase]) || levelBaseTimeForUpgrade(&levelScene->bases[sourceBase], plan->param0) < 0.0f) {
                plan->planType = AIPlanTypeNone;
            }
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
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) != planner->teamNumber ||
                levelBaseGetTeam(&levelScene->bases[plan->param0]) == planner->teamNumber || 
                levelScene->bases[plan->targetBase].defaultComand == MinionCommandAttack;
        case AIPlanTypeUpgrade:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) != planner->teamNumber ||
                levelBaseIsBeingUpgraded(&levelScene->bases[plan->targetBase]);
        default:
            return 0;
    }
}

int aiPlannerIsPlanStillValid(struct LevelScene* levelScene, struct AIPlanner* planner, struct AIPlan* plan) {
    switch (plan->planType) {
        case AIPlanTypeAttackBase:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) != planner->teamNumber; 
        case AIPlanTypeAttackBaseWithBase:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) == planner->teamNumber && 
                levelBaseGetTeam(&levelScene->bases[plan->param0]) != planner->teamNumber; 
        case AIPlanTypeUpgrade:
            return levelBaseGetTeam(&levelScene->bases[plan->targetBase]) == planner->teamNumber;
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
    planner->thinkingTimer = aiPlannerThinkingTime(planner);

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

void aiPlannerInit(struct AIPlanner* planner, unsigned teamNumber, unsigned baseCount, float difficulty) {
    zeroMemory(planner, sizeof(struct AIPlanner));
    planner->thinkingTimer = aiPlannerThinkingTime(planner);
    planner->teamNumber = teamNumber;
    planner->baseCount = baseCount;
    planner->difficulty = difficulty;
    planner->basesCoveredByPlan = malloc(sizeof(struct AIPlan*) * baseCount);
    zeroMemory(planner->basesCoveredByPlan, sizeof(struct AIPlan*) * baseCount);
    planner->baseMultiplyer = malloc(sizeof(float) * baseCount);
    zeroMemory(planner->baseMultiplyer, sizeof(float) * baseCount);

    for (unsigned i = 0; i < baseCount; ++i) {
        planner->baseMultiplyer[i] = 1.0f;
    }
}

void aiPlannerUpdate(struct LevelScene* levelScene, struct AIPlanner* planner) {
    // make sure next plan is up to date
    aiPlannerScorePlan(levelScene, planner, &planner->nextPlan);

    int plansPerFrame = (int)mathfLerp(EASY_PLANS_PER_FRAME, HARD_PLANS_PER_FRAME, planner->difficulty);

    for (unsigned i = 0; i < plansPerFrame; ++i) {
        struct AIPlan newPlan;
        aiPlannerComeUpWithPlan(levelScene, planner, &newPlan);
        if (aiPlannerIsPlanStillValid(levelScene, planner, &newPlan) && !aiPlannerDoesPlanConflict(levelScene, planner, &newPlan)) {
            aiPlannerScorePlan(levelScene, planner, &newPlan);
            
            if (planner->nextPlan.planType == AIPlanTypeNone || (newPlan.planType != AIPlanTypeNone && planner->nextPlan.estimatedCost > newPlan.estimatedCost)) {
                planner->nextPlan = newPlan;
            }
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
        planner->thinkingTimer = aiPlannerThinkingTime(planner);
    } else if (!planner->currentPlan && planner->thinkingTimer <= 0) {
        aiPlannerImplementNextPlan(levelScene, planner);
    } else if (planner->thinkingTimer > 0) {
        // thinking timer is used to ensure the AI comes up with a few 
        // plans before trying to execute the best option
        planner->thinkingTimer -= gTimeDelta;
    }
}

struct LevelBase* aiPlannerGetTargetBase(struct LevelScene* levelScene, struct AIPlanner* planner) {
    if (!planner->currentPlan) {
        return 0;
    }

    switch (planner->currentPlan->planType) {
        case AIPlanTypeAttackBase:
        case AIPlanTypeAttackBaseWithBase:
        case AIPlanTypeUpgrade:
            return &levelScene->bases[planner->currentPlan->targetBase];
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

void aiPlannerReset(struct AIPlanner* planner) {
    if (planner->currentPlan && planner->currentPlan->planType == AIPlanTypeAttackBase) {
        planner->baseMultiplyer[planner->currentPlan->targetBase] *= 1.1f;
    }

    planner->currentPlan = 0;
    planner->nextPlan.planType = AIPlanTypeNone;
    planner->thinkingTimer = aiPlannerThinkingTime(planner);
    zeroMemory(&planner->activePlans, sizeof(planner->activePlans));
    zeroMemory(planner->basesCoveredByPlan, sizeof(struct AIPlan*) * planner->baseCount);
}