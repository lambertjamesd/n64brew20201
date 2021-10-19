
#include <ultra64.h>

#include "minion.h"
#include "util/rom.h"
#include "graphics/gfx.h"
#include "util/time.h"
#include <stdlib.h>

#include "sk64/skelatool_animation_clip.h"
#include "../data/models/characters.h"
#include "scene_management.h"

#include "../data/models/example/geometry_animdef.inc.h"

struct MinionDef {
    Gfx* dl;
    unsigned boneCount;
    struct Transform* defaultPose;
};

struct MinionDef gMinionDefs[] = {
    {output_model_gfx, OUTPUT_DEFAULT_BONES_COUNT, output_default_bones},
};

void minionSetup() {
    output_animations[0].firstChunk = CALC_ROM_POINTER(character_animations, output_animations[0].firstChunk);
}

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at, unsigned char sourceBaseId) {
    minion->transform = *at;
    minion->minionType = type;
    minion->minionFlags = MinionFlagsActive;
    minion->sourceBaseId = sourceBaseId;

    minion->transform.scale.x *= 0.1f;
    minion->transform.scale.y *= 0.1f;
    minion->transform.scale.z *= 0.1f;

    quatAxisAngle(&gUp, M_PI * 2.0f * rand() / RAND_MAX, &minion->transform.rotation);
    
    // skInitObject(
    //     &minion->armature, 
    //     gMinionDefs[type].dl, 
    //     gMinionDefs[type].boneCount, 
    //     CALC_ROM_POINTER(character_animations, gMinionDefs[type].defaultPose)
    // );

    // skAnimatorInit(&minion->animator, gMinionDefs[type].boneCount);
    // skAnimatorRunClip(&minion->animator, &output_animations[0], SKAnimatorFlagsLoop);
}

void minionRender(struct Minion* minion, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&minion->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, DogMinion_Dog_001_mesh);
    // skRenderObject(&minion->armature, renderState);
    gSPPopMatrix(renderState->dl++, 1);
}
    
void minionUpdate(struct Minion* minion) {
    // skAnimatorUpdate(&minion->animator, &minion->armature, 0.5f);
}

void minionCleanup(struct Minion* minion) {
    if (minion->minionFlags & MinionFlagsActive) {
        minion->minionFlags = 0;
        levelBaseReleaseMinion(&gCurrentLevel.bases[minion->sourceBaseId]);
    }
}