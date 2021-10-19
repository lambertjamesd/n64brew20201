#ifndef _MINION_H
#define _MINION_H

#include "math/transform.h"
#include "sk64/skelatool_object.h"
#include "sk64/skelatool_animation.h"
#include "graphics/render_state.h"
#include "factionentity.h"

enum MinionFlags {
    MinionFlagsActive = (1 << 0),
};

enum MinionType {
    MinionTypeMelee,
    MinionTypeCount,
};

enum MinionCurrentTask {
    MinionActiontypeFollow,
    MinionActiontypeAttack,
    MinionActiontypeDefend,
};

struct Minion {
    struct FactionEntity faction;
    struct Transform transform;
    unsigned char minionFlags;
    unsigned char minionType;
    unsigned char sourceBaseId;
    unsigned char currentTask;

    float currentActionDuration;

    // struct SKObject armature;
    // struct SKAnimator animator;
    
};

void minionSetup();

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at, unsigned char baseId);
void minionRender(struct Minion* minion, struct RenderState* renderState);
void minionUpdate(struct Minion* minion);
void minionCleanup(struct Minion* minion);

#endif