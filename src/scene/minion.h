#ifndef _MINION_H
#define _MINION_H

#include "math/transform.h"
#include "sk64/skelatool_armature.h"
#include "sk64/skelatool_animation.h"
#include "graphics/render_state.h"
#include "teamentity.h"
#include "collision/dynamicscene.h"

#define MINION_DEFENSE_RADIUS  10.0f
#define MINION_GFX_PER_MINION  3

enum MinionFlags {
    MinionFlagsActive = (1 << 0),
};

enum MinionType {
    MinionTypeMelee,
    MinionTypeCount,
};

enum MinionCommand {
    MinionCommandFollow,
    MinionCommandAttack,
    MinionCommandDefend,
};

struct Minion {
    struct TeamEntity team;
    struct Transform transform;
    struct Vector3 velocity;
    struct TeamEntity* currentTarget;
    struct Vector3 defensePoint;
    unsigned char minionFlags;
    unsigned char minionType;
    unsigned char sourceBaseId;
    unsigned char currentCommand;

    struct DynamicSceneEntry* collider;

    // struct SKArmature armature;
    // struct SKAnimator animator;
    
};

void minionSetup();

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at, unsigned char baseId, unsigned team);
void minionRender(struct Minion* minion, struct RenderState* renderState);
void minionUpdate(struct Minion* minion);
void minionCleanup(struct Minion* minion);
void minionIssueCommand(struct Minion* minion, enum MinionCommand command);

#endif