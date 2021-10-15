
#include <ultra64.h>

#include "minion.h"
#include "util/rom.h"

#include "../../data/models/example/geometry.h"

struct MinionDef {
    Gfx* dl;
    unsigned boneCount;
    struct Transform* defaultPose;
};

struct MinionDef gMinionDefs[] = {
    {output_model_gfx, OUTPUT_DEFAULT_BONES_COUNT, output_default_bones},
};

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at) {
    minion->transform = *at;
    minion->minionType = type;
    minion->minionFlags = MinionFlagsActive;
    
    skInitObject(&minion->armature, gMinionDefs[type].dl, gMinionDefs[type].boneCount, gMinionDefs[type].defaultPose);
}

void minionRender(struct Minion* minion, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&minion->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    skRenderObject(&minion->armature, renderState);
    gSPPopMatrix(renderState->dl++, 1);
}