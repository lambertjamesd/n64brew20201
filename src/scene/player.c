
#include "player.h"
#include "../data/models/characters.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "game_defs.h"
#include "util/time.h"

#define PLAYER_MOVE_SPEED           16.0f
#define PLAYER_AIR_SPEED            (PLAYER_MOVE_SPEED * 0.8f)
#define PLAYER_MOVE_ACCELERATION    10.0f
#define PLAYER_STOP_ACCELERATION    30.0f
#define PLAYER_AIR_ACCELERATION     4.0f

#define PLAYER_JUMP_ACCEL           (-GRAVITY * 0.5f)

#define PLAYER_JUMP_IMPULSE         12.0f

#define PLAYER_AIR_MAX_ROTATE_SEC   (90.0f * (M_PI / 180.0f))
#define PLAYER_MAX_ROTATE_SEC       (270.0f * (M_PI / 180.0f))

struct CollisionCircle gPlayerCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * 0.2f,
};

void playerStateWalk(struct Player* player, struct PlayerInput* input);

void playerInit(struct Player* player, unsigned team, struct Vector2* at) {
    player->team.entityType = FactionEntityTypePlayer;
    player->team.entityFaction = team;
    transformInitIdentity(&player->transform);
    player->transform.position.x = at->x;
    player->transform.position.z = at->y;

    player->velocity = gZeroVec;
    player->rightDir = gRight2;

    player->armature.displayList = doglow_DogLow_mesh;

    player->collider = dynamicSceneNewEntry(
        &gPlayerCollider.shapeCommon,
        player,
        at,
        0,
        DynamicSceneEntryHasFaction,
        CollisionLayersTangible | CollisionLayersBase
    );

    player->state = playerStateWalk;
}

void playerRotateTowardsInput(struct Player* player, struct PlayerInput* input, float rotationRate) {
    struct Vector2 rightDir;
    rightDir.x = -input->targetWorldDirection.z;
    rightDir.y = -input->targetWorldDirection.x;

    if (rightDir.x != 0.0f || rightDir.y != 0.0f) {
        vector2Normalize(&rightDir, &rightDir);
        struct Vector2 maxRotate;
        vector2ComplexFromAngle(rotationRate * gTimeDelta, &maxRotate);

        vector2RotateTowards(&player->rightDir, &rightDir, &maxRotate, &player->rightDir);
        quatAxisComplex(&gUp, &player->rightDir, &player->transform.rotation);
    }
}

void playerAccelerateTowards(struct Player* player, struct Vector3* targetDirection, float speed, float forwardAccell, float backwardAccell) {
    struct Vector3 targetVelocity;
    vector3Scale(targetDirection, &targetVelocity, speed);
    float acceleration;

    if (vector3Dot(&targetVelocity, &player->velocity) > 0.0f) {
        acceleration = forwardAccell;
    } else {
        acceleration = backwardAccell;
    }

    vector3MoveTowards(&player->velocity, &targetVelocity, acceleration * gTimeDelta, &player->velocity);
}

void playerStateFreefall(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_AIR_MAX_ROTATE_SEC);
    playerAccelerateTowards(player, &input->targetWorldDirection, PLAYER_AIR_SPEED, PLAYER_AIR_ACCELERATION, PLAYER_AIR_ACCELERATION);


    if (player->transform.position.y <= 0.0f && player->velocity.y <= 0.0f) {
        player->state = playerStateWalk;
    }
}

void playerStateJump(struct Player* player, struct PlayerInput* input) {
    if (input->actionFlags & PlayerInputActionsJump) {
        player->velocity.y += PLAYER_JUMP_ACCEL * gTimeDelta;
        playerStateFreefall(player, input);
    } else {
        player->state = playerStateFreefall;
    }
}

void playerStateWalk(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_MAX_ROTATE_SEC);
    playerAccelerateTowards(player, &input->targetWorldDirection, PLAYER_MOVE_SPEED, PLAYER_MOVE_ACCELERATION, PLAYER_STOP_ACCELERATION);

    if (input->actionFlags & PlayerInputActionsJump) {
        player->velocity.y = PLAYER_JUMP_IMPULSE;
        player->state = playerStateJump;
    }
}

void playerUpdate(struct Player* player, struct PlayerInput* input) {
    player->state(player, input);

    player->velocity.y += GRAVITY * gTimeDelta;
    vector3AddScaled(&player->transform.position, &player->velocity, gTimeDelta * SCENE_SCALE, &player->transform.position);

    if (player->transform.position.y < FLOOR_HEIGHT) {
        player->transform.position.y = FLOOR_HEIGHT;
        player->velocity.y = 0.0f;
    }
}

void playerRender(struct Player* player, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&player->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, player->armature.displayList);
    gSPPopMatrix(renderState->dl++, 1);
}