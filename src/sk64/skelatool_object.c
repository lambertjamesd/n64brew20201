
#include "skelatool_object.h"
#include "skelatool_defs.h"
#include "util/memory.h"
#include "util/rom.h"

void skInitObject(struct SkelatoolObject* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose) {
    object->displayList = displayList;
    object->numberOfBones = numberOfBones;
    object->boneTransforms = malloc(sizeof(Mtx) * numberOfBones);
    romCopy((void*)initialPose, (void*)object->boneTransforms, numberOfBones);
}

void skCleanupObject(struct SkelatoolObject* object) {
    free(object->boneTransforms);
    object->boneTransforms = 0;
    object->numberOfBones = 0;
}

void skRenderObject(struct SkelatoolObject* object, struct RenderState* intoState) {
    Mtx* boneMatrices = renderStateRequestMatrices(intoState, object->numberOfBones);

    if (boneMatrices) {
        skCalculateTransforms(object, boneMatrices);
        gSPSegment(intoState->dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(boneMatrices));
        gSPDisplayList(intoState->dl++, object->displayList);
    }
}

void skCalculateTransforms(struct SkelatoolObject* object, Mtx* into) {
    for (int i = 0; i < object->numberOfBones; ++i) {
        transformToMatrixL(&object->boneTransforms[i], &into[i]);
    }
}