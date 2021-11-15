#ifndef AI_PLANNER_H
#define AI_PLANNER_H

struct LevelScene;

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
    struct AIPlan** basesCoveredByPlan;
};

#endif