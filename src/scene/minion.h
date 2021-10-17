#ifndef _MINION_H
#define _MINION_H

#include "math/transform.h"
#include "sk64/skelatool_object.h"
#include "sk64/skelatool_animation.h"
#include "graphics/render_state.h"

enum MinionFlags {
    MinionFlagsActive = (1 << 0),
};

enum MinionType {
    MinionTypeMelee,
    MinionTypeCount,
};

struct Minion {
    struct Transform transform;
    unsigned short minionFlags;
    unsigned short minionType;
    struct SKObject armature;
    struct SKAnimator animator;
};

void minionSetup();

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at);
void minionRender(struct Minion* minion, struct RenderState* renderState);
void minionUpdate(struct Minion* minion);

#endif