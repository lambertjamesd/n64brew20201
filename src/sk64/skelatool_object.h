#ifndef __SKELATOOL_OBJECT_H
#define __SKELATOOL_OBJECT_H

#include <ultra64.h>
#include "math/transform.h"
#include "graphics/render_state.h"

struct SkelatoolObject {
    Gfx* displayList;
    struct Transform* boneTransforms;
    u32 numberOfBones;
};

void skInitObject(struct SkelatoolObject* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose);
void skRenderObject(struct SkelatoolObject* object, struct RenderState* intoState);
void skCalculateTransforms(struct SkelatoolObject* object, Mtx* into);
void skCleanupObject(struct SkelatoolObject* object);

#endif