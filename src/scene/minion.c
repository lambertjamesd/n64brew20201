
#include <ultra64.h>

#include "minion.h"
#include "util/rom.h"
#include "graphics/gfx.h"
#include "util/time.h"
#include <stdlib.h>

#include "sk64/skelatool_animation_clip.h"
#include "../data/models/characters.h"

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

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at) {
    minion->transform = *at;
    minion->minionType = type;
    minion->minionFlags = MinionFlagsActive;

    minion->transform.scale.x *= 0.1f;
    minion->transform.scale.y *= 0.1f;
    minion->transform.scale.z *= 0.1f;

    minion->currentAction = MinionActiontypeMove;
    minion->currentActionDuration = 1.0f;

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

    switch (minion->currentAction) {
        case MinionActiontypeMove:
        {
            struct Vector3 forward;
            vector3Scale(&gForward, &forward, gTimeDelta * 0.1f);
            quatMultVector(&minion->transform.rotation, &forward, &forward);
            vector3Add(&minion->transform.position, &forward, &minion->transform.position);
            break;
        }
        case MinionActiontypeTurnLeft:
        {
            struct Quaternion rotateDelta;
            quatAxisAngle(&gUp, gTimeDelta, &rotateDelta);
            quatMultiply(&minion->transform.rotation, &rotateDelta, &minion->transform.rotation);
            break;
        }
        case MinionActiontypeTurnRight:
        {
            struct Quaternion rotateDelta;
            quatAxisAngle(&gUp, -gTimeDelta, &rotateDelta);
            quatMultiply(&minion->transform.rotation, &rotateDelta, &minion->transform.rotation);
            break;
        }
    }
    
    minion->currentActionDuration -= gTimeDelta;
    
    if (minion->currentActionDuration <= 0.0) {
        if (minion->currentAction == MinionActiontypePause) {
            minion->currentAction = rand() % 3 + 1;
        } else {
            minion->currentAction = MinionActiontypePause;
        }

        minion->currentActionDuration = 1.0f + (rand() % 100) / 100.0f;
    }

    // skAnimatorUpdate(&minion->animator, &minion->armature, 0.5f);
}