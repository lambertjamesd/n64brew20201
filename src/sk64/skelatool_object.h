#ifndef __SKELATOOL_OBJECT_H
#define __SKELATOOL_OBJECT_H

#include <ultra64.h>
#include "math/transform.h"

struct SkelatoolObject {
    Gfx* displayList;
    struct Transform* boneTransforms;
    Mtx* boneMatrices;
    u32 numberOfBones;
};

void skRenderObject(struct SkelatoolObject* object, Gfx** intoDL);
void skUpdateTransforms(struct SkelatoolObject* object);
void skInitObject(struct SkelatoolObject* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose);
void skCleanupObject(struct SkelatoolObject* object);

#endif