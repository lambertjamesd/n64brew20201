#ifndef AI_PLANNER_H
#define AI_PLANNER_H

struct LevelScene;

#define MINIMUM_THIKING_FRAMES  30

enum AIPlanType {
    AIPlanTypeNone,
    AIPlanTypeAttackBase,
    AIPlanTypeAttackBaseWithBase,
    // AIPlanTypeUpgrade,
    // AIPlanDefend,
    AIPlanCount,
};

struct AIPlan {
    enum AIPlanType planType;
    short targetBase;
    short neededMinions;
    short param0;
    float estimatedCost;
};

#define MAX_ACTIVE_PLANS    4

struct AIPlanner {
    struct AIPlan* currentPlan;
    struct AIPlan activePlans[MAX_ACTIVE_PLANS];
    struct AIPlan nextPlan;
    unsigned short teamNumber;
    unsigned short baseCount;
    unsigned short thinkingTimer;
    struct AIPlan** basesCoveredByPlan;
};

void aiPlannerInit(struct AIPlanner* planner, unsigned teamNumber, unsigned baseCount);
void aiPlannerUpdate(struct LevelScene* levelScene, struct AIPlanner* planner);
struct LevelBase* aiPlannerGetTargetBase(struct LevelScene* levelScene, struct AIPlanner* planner);
struct Vector3* aiPlannerGetTarget(struct LevelScene* levelScene, struct AIPlanner* planner);

#endif