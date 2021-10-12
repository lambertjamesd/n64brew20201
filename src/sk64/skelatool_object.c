
#include "skelatool_object.h"
#include "skelatool_defs.h"
#include "util/memory.h"

void skInitObject(struct SkelatoolObject* object, Gfx* displayList, u32 numberOfBones, struct Transform* initialPose) {
    object->displayList = displayList;
    object->numberOfBones = numberOfBones;
    object->boneMatrices = malloc(sizeof(struct Transform) * numberOfBones);
    object->boneTransforms = malloc(sizeof(Mtx) * numberOfBones);
}

void skCleanupObject(struct SkelatoolObject* object) {
    free(object->boneMatrices);
    free(object->boneTransforms);
    object->boneMatrices = 0;
    object->boneTransforms = 0;
    object->numberOfBones = 0;
}

void skRenderObject(struct SkelatoolObject* object, Gfx** intoDL) {
    Gfx* dl = *intoDL;

    gSPSegment(dl++, MATRIX_TRANSFORM_SEGMENT,  osVirtualToPhysical(object->boneTransforms));
    gSPDisplayList(dl++, object->displayList);

    *intoDL = dl;
}

void skUpdateTransforms(struct SkelatoolObject* object) {
    for (int i = 0; i < object->numberOfBones; ++i) {
        transformToMatrixL(&object->boneTransforms[i], &object->boneMatrices[i]);
    }
}