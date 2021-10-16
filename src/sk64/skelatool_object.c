
#include "skelatool_object.h"
#include "skelatool_defs.h"
#include "util/memory.h"
#include "util/rom.h"

void skInitObject(struct SKObject* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose) {
    object->displayList = displayList;
    object->numberOfBones = numberOfBones;
    object->boneTransforms = malloc(sizeof(Mtx) * numberOfBones);
    if (initialPose) {
        romCopy((void*)initialPose, (void*)object->boneTransforms, sizeof(Mtx) * numberOfBones);
    }
}

void skCleanupObject(struct SKObject* object) {
    free(object->boneTransforms);
    object->boneTransforms = 0;
    object->numberOfBones = 0;
}

void skRenderObject(struct SKObject* object, struct RenderState* intoState) {
    if (!object->displayList) {
        return;
    }

    Mtx* boneMatrices = renderStateRequestMatrices(intoState, object->numberOfBones);

    if (boneMatrices) {
        skCalculateTransforms(object, boneMatrices);
        gSPSegment(intoState->dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(boneMatrices));
        gSPDisplayList(intoState->dl++, object->displayList);
    }
}

void skCalculateTransforms(struct SKObject* object, Mtx* into) {
    for (int i = 0; i < object->numberOfBones; ++i) {
        transformToMatrixL(&object->boneTransforms[i], &into[i]);
    }
}