
#include <ultra64.h>

#include "minion.h"
#include "util/rom.h"
#include "graphics/gfx.h"
#include "util/time.h"
#include <stdlib.h>

#include "sk64/skelatool_animation_clip.h"
#include "../data/models/characters.h"
#include "scene_management.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "scene_management.h"

#include "../data/models/example/geometry_animdef.inc.h"

#define MINION_FOLLOW_DIST  2.0f
#define MINION_MOVE_SPEED   (PLAYER_MOVE_SPEED * 0.9f)
#define MINION_ACCELERATION PLAYER_MOVE_ACCELERATION

struct CollisionCircle gMinionCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * 0.3f,
};

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

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at, unsigned char sourceBaseId, unsigned team) {
    minion->team.teamNumber = team;
    minion->team.entityType = TeamEntityTypeMinion;

    minion->transform = *at;
    minion->minionType = type;
    minion->minionFlags = MinionFlagsActive;
    minion->sourceBaseId = sourceBaseId;
    minion->velocity = gZeroVec;

    minion->transform.scale.x *= 0.2f;
    minion->transform.scale.y *= 0.2f;
    minion->transform.scale.z *= 0.2f;

    struct Vector2 position;

    position.x = minion->transform.position.x;
    position.y = minion->transform.position.z;

    minion->collider = dynamicSceneNewEntry(
        &gMinionCollider.shapeCommon, 
        minion, 
        &position,
        teamEntityCorrectOverlap,
        DynamicSceneEntryHasTeam,
        CollisionLayersTangible | CollisionLayersBase
    );

    quatAxisAngle(&gUp, M_PI * 2.0f * rand() / RAND_MAX, &minion->transform.rotation);

    minion->currentAction = MinionActiontypeFollow;
}

void minionRender(struct Minion* minion, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&minion->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, DogMinion_Dog_001_mesh);
    gSPPopMatrix(renderState->dl++, 1);
}
    
void minionUpdate(struct Minion* minion) {
    struct Vector3* target;
    float minDistance = 0.0f;

    switch (minion->currentAction) {
        case MinionActiontypeFollow:
            target = &gCurrentLevel.players[minion->team.teamNumber].transform.position;
            minDistance = MINION_FOLLOW_DIST * SCENE_SCALE;
            break;
    }

    struct Vector3 targetVelocity = gZeroVec;

    if (target) {
        struct Vector3 offset;
        vector3Sub(target, &minion->transform.position, &offset);

        float distSqr = vector3MagSqrd(&offset);

        if (distSqr > minDistance * minDistance) {
            vector3Scale(&offset, &offset, 1.0f / sqrtf(distSqr));
            vector3Scale(&offset, &targetVelocity, MINION_MOVE_SPEED);
        }
    }

    vector3MoveTowards(&minion->velocity, &targetVelocity, MINION_ACCELERATION * gTimeDelta, &minion->velocity);
    vector3AddScaled(&minion->transform.position, &minion->velocity, SCENE_SCALE * gTimeDelta, &minion->transform.position);

    minion->collider->center.x = minion->transform.position.x;
    minion->collider->center.y = minion->transform.position.z;
}

void minionCleanup(struct Minion* minion) {
    if (minion->minionFlags & MinionFlagsActive) {
        minion->minionFlags = 0;
        levelBaseReleaseMinion(&gCurrentLevel.bases[minion->sourceBaseId]);
    }
}