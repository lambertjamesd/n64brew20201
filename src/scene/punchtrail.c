#include "punchtrail.h"
#include "sk64/skelatool_defs.h"
#include "game_defs.h"

void punchTrailInit(struct PunchTrail* punchTrail, struct Vector3* start, float scale) {
    for (unsigned i = 0; i < PUNCHTRAIL_DEFAULT_BONES_COUNT; ++i) {
        quatIdent(&punchTrail->segmentTransforms[i].rotation);
        punchTrail->segmentTransforms[i].position = *start;
        vector3Scale(&gOneVec, &punchTrail->segmentTransforms[i].scale, scale);
    }
}

void punchTrailUpdate(struct PunchTrail* punchTrail, struct Vector3* next) {
    for (unsigned i = PUNCHTRAIL_DEFAULT_BONES_COUNT - 1; i >= 1; --i) {
        punchTrail->segmentTransforms[i].position = punchTrail->segmentTransforms[i-1].position;
        punchTrail->segmentTransforms[i].rotation = punchTrail->segmentTransforms[i-1].rotation;
    }

    struct Vector3 dir;
    vector3Sub(next, &punchTrail->segmentTransforms[0].position, &dir);
    punchTrail->segmentTransforms[0].position = *next;
    if (vector3MagSqrd(&dir) > 0.00001f) {
        quatLook(&dir, &gUp, &punchTrail->segmentTransforms[0].rotation);
    }
}

void punchTrailRender(struct PunchTrail* punchTrail, struct RenderState* renderState, struct Coloru8 color) {
    Mtx* matrices = renderStateRequestMatrices(renderState, PUNCHTRAIL_DEFAULT_BONES_COUNT);

    for (unsigned i = 0; i < PUNCHTRAIL_DEFAULT_BONES_COUNT; ++i) {
        transformToMatrixL(&punchTrail->segmentTransforms[i], &matrices[i]);
    }

    gDPSetPrimColor(renderState->transparentDL++, 255, 255, color.r, color.g, color.b, color.a);
    gSPSegment(renderState->transparentDL++, MATRIX_TRANSFORM_SEGMENT, osVirtualToPhysical(matrices));
    gSPDisplayList(renderState->transparentDL++, punchtrail_model_gfx);
}

struct Vector3* punchTrailHeadPosition(struct PunchTrail* punchTrail) {
    return &punchTrail->segmentTransforms[0].position;
}