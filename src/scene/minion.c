
#include <ultra64.h>

#include "minion.h"
#include "util/rom.h"
#include "graphics/gfx.h"
#include "util/time.h"

#include "sk64/skelatool_animation_clip.h"
#include "../data/models/characters.h"

struct SKAnimationHeader animationTestHeaders[] = {
    {
        .firstChunkSize = 176,
        .ticksPerSecond = 25,
        .maxTicks = 50,
        .firstChunk = (struct SKAnimationChunk*)output_default_idle_animation,
    }
};

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

    animationTestHeaders[0].firstChunk = CALC_ROM_POINTER(character_animations, animationTestHeaders[0].firstChunk);
    
    skInitObject(
        &minion->armature, 
        gMinionDefs[type].dl, 
        gMinionDefs[type].boneCount, 
        CALC_ROM_POINTER(character_animations, gMinionDefs[type].defaultPose)
    );

    skAnimatorInit(&minion->animator, gMinionDefs[type].boneCount);
    skAnimatorRunClip(&minion->animator, &animationTestHeaders[0], SKAnimatorFlagsLoop);
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
    
void minionUpdate(struct Minion* minion) {
    struct Quaternion rotation;
    quatAxisAngle(&gForward, 0.5, &rotation);

    // for (unsigned i = 0; i < minion->armature.numberOfBones; ++i) {
    //     minion->armature.boneTransforms[i].rotation = rotation;
    // }

    skAnimatorUpdate(&minion->animator, &minion->armature);
}