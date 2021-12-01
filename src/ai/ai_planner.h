#ifndef AI_PLANNER_H
#define AI_PLANNER_H

struct LevelScene;

enum AIPlanType {
    AIPlanTypeNone,
    AIPlanTypeAttackBase,
    AIPlanTypeAttackBaseWithBase,
    AIPlanTypeUpgrade,
    // AIPlanDefend,
    AIPlanCount,
};

struct AIPlan {
    enum AIPlanType planType;
    // base the player needs to move to
    // in order to complete plan
    short targetBase;
    short neededMinions;
    short param0;
    float estimatedCost;
};

enum TeamBaseType {
    CurrentTeam,
    EnemyTeam,
    AnyTeam,
};

#define MAX_ACTIVE_PLANS    8

struct AIPlanner {
    struct AIPlan* currentPlan;
    struct AIPlan activePlans[MAX_ACTIVE_PLANS];
    struct AIPlan nextPlan;
    unsigned short teamNumber;
    unsigned short baseCount;
    struct AIPlan** basesCoveredByPlan;
    float* baseMultiplyer;
    float difficulty;
    float thinkingTimer;
};

int aiPlannerDoesTeamMatch(unsigned fromTeam, unsigned toTeam, enum TeamBaseType teamType);

void aiPlannerInit(struct AIPlanner* planner, unsigned teamNumber, unsigned baseCount, float difficulty);
void aiPlannerUpdate(struct LevelScene* levelScene, struct AIPlanner* planner);
struct LevelBase* aiPlannerGetTargetBase(struct LevelScene* levelScene, struct AIPlanner* planner);
struct Vector3* aiPlannerGetTarget(struct LevelScene* levelScene, struct AIPlanner* planner);
void aiPlannerReset(struct AIPlanner* planner);
int aiPlannerFindNearestBaseToPoint(struct LevelScene* levelScene, struct Vector3* from, unsigned team, enum TeamBaseType teamType, float* distanceSqr);

#endif