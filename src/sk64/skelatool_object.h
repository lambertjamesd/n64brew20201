#ifndef __SKELATOOL_OBJECT_H
#define __SKELATOOL_OBJECT_H

#include <ultra64.h>
#include "math/transform.h"
#include "graphics/render_state.h"

struct SKObject {
    Gfx* displayList;
    struct Transform* boneTransforms;
    u32 numberOfBones;
};

void skInitObject(struct SKObject* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose);
void skRenderObject(struct SKObject* object, struct RenderState* intoState);
void skCalculateTransforms(struct SKObject* object, Mtx* into);
void skCleanupObject(struct SKObject* object);

#endif