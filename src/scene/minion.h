#ifndef _MINION_H
#define _MINION_H

#include "math/transform.h"
#include "sk64/skelatool_armature.h"
#include "sk64/skelatool_animator.h"
#include "graphics/render_state.h"
#include "teamentity.h"
#include "ai/ai_pathfinder.h"
#include "collision/dynamicscene.h"
#include "damagehandler.h"

#define MINION_DEFENSE_RADIUS  (10.0f * SCENE_SCALE)
#define MINION_FOLLOW_RADIUS  (20.0f * SCENE_SCALE)
#define MINION_GFX_PER_MINION  7
#define MINION_COLLIDE_RADIUS (SCENE_SCALE * 0.4f)

enum MinionFlags {
    MinionFlagsActive = (1 << 0),
    MinionFlagsAttacking = (1 << 1),
    MinionFlagsAttacked = (1 << 2),
};

enum MinionType {
    MinionTypeMelee,
    MinionTypeCount,
};

enum MinionCommand {
    MinionCommandFollow,
    MinionCommandAttack,
    MinionCommandDefend,
    MinionCommandCount,
};

struct Minion {
    struct TeamEntity team;
    struct Transform transform;
    struct Vector3 velocity;
    struct TeamEntity* currentTarget;
    struct TeamEntity* attackTarget;
    struct Vector3 defensePoint;
    unsigned char minionFlags;
    unsigned char minionType;
    unsigned char sourceBaseId;
    unsigned char currentCommand;
    unsigned char followingPlayer;
    struct DamageHandler damageHandler;
    struct Pathfinder pathfinder;

    struct DynamicSceneEntry* collider;

    struct SKAnimator animator;
    struct Transform animationTransform;
    float attackTimer;
};

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at, unsigned char baseId, unsigned team, enum MinionCommand defualtCommand, unsigned followPlayer);
void minionRender(struct Minion* minion, struct RenderState* renderState);
void minionUpdate(struct Minion* minion);
void minionCleanup(struct Minion* minion);
void minionIssueCommand(struct Minion* minion, enum MinionCommand command, unsigned fromPlayer);
void minionSetAttackTarget(struct Minion* minion, struct TeamEntity* target);
void minionApplyDamage(struct Minion* minion, float amount);
int minionIsAlive(struct Minion* minion);
void minionSetTarget(struct Minion* minion, struct TeamEntity* value);

#endif