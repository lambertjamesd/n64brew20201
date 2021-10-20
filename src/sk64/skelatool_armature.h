#ifndef __SKELATOOL_OBJECT_H
#define __SKELATOOL_OBJECT_H

#include <ultra64.h>
#include "math/transform.h"
#include "graphics/render_state.h"

struct SKArmature {
    Gfx* displayList;
    struct Transform* boneTransforms;
    u32 numberOfBones;
};

void skInitObject(struct SKArmature* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose);
void skRenderObject(struct SKArmature* object, struct RenderState* intoState);
void skCalculateTransforms(struct SKArmature* object, Mtx* into);
void skCleanupObject(struct SKArmature* object);

#endif