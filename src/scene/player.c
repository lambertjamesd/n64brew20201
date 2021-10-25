
#include "player.h"
#include "../data/models/characters.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "game_defs.h"
#include "util/time.h"
#include "levelbase.h"
#include "controls/controller.h"
#include "level_scene.h"
#include "scene_management.h"
#include "menu/basecommandmenu.h"
#include "util/rom.h"
#include "graphics/gfx.h"

#define PLAYER_ANIMATION_ATTACK_START(sourceId)   ((sourceId) << 8)
#define PLAYER_IS_ATTACK_EVENT(eventId)         (((eventId) & 0xFF) == 0)
#define PLAYER_ATTACK_EVENT_GET_SOURCE(eventId)     ((eventId) >> 8)

#define PLAYER_ATTACK_END_ID                       0x1

struct PlayerAttackInfo mAttacks[] = {
    {
        DOGLOW_ARM1_BONE, 
        {0.0f, 0.0f, 6.5f * SCENE_SCALE}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
    },
    {
        DOGLOW_ARM2_BONE, 
        {0.0f, 0.0f, 6.5f * SCENE_SCALE}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
    },
};

#include "../data/models/doglow/geometry_animdef.inc.h"

#define PLAYER_AIR_SPEED            (PLAYER_MOVE_SPEED * 0.8f)
#define PLAYER_STOP_ACCELERATION    50.0f
#define PLAYER_AIR_ACCELERATION     10.0f

#define PLAYER_JUMP_ACCEL           (-GRAVITY * 0.5f)

#define PLAYER_JUMP_IMPULSE         12.0f

#define PLAYER_AIR_MAX_ROTATE_SEC   (90.0f * (M_PI / 180.0f))
#define PLAYER_MAX_ROTATE_SEC       (360.0f * (M_PI / 180.0f))

struct CollisionCircle gPlayerCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * 0.5f,
};

struct Vector3 gRecallOffset = {0.0f, 0.0f, -4.0 * SCENE_SCALE};

void playerAttackOverlap(struct DynamicSceneOverlap* overlap) {
    struct Player* player = (struct Player*)overlap->thisEntry->data;

    if ((overlap->otherEntry->flags & DynamicSceneEntryHasTeam) != 0 && player->attackInfo) {
        struct TeamEntity* entityB = (struct TeamEntity*)overlap->otherEntry->data;
        teamEntityApplyDamage(entityB, player->attackInfo->damage);
    }
}

void playerCalculateAttackLocation(struct Player* player, struct PlayerAttackInfo* attackInfo, struct Vector2* output) {
    struct Vector3 output3D;
    skCalculateBonePosition(&player->armature, attackInfo->boneIndex, &attackInfo->localPosition, &output3D);
    transformPoint(&player->transform, &output3D, &output3D);
    output->x = output3D.x;
    output->y = output3D.z;
}

void playerStartAttack(struct Player* player, struct PlayerAttackInfo* attackInfo) {
    if (!player->attackCollider) {
        struct Vector2 position;
        playerCalculateAttackLocation(player, attackInfo, &position);
        player->attackCollider = dynamicSceneNewEntry(
            &attackInfo->collisionCircle.shapeCommon, 
            player,
            &position,
            playerAttackOverlap,
            DynamicSceneEntryIsTrigger,
            CollisionLayersAllTeams ^ COLLISION_LAYER_FOR_TEAM(player->team.teamNumber)
        );
        player->attackInfo = attackInfo;
    }
}

void playerEndAttack(struct Player* player) {
    if (player->attackCollider) {
        dynamicSceneDeleteEntry(player->attackCollider);
        player->attackInfo = 0;
        player->attackCollider = 0;
    }
}

void playerAnimationEvent(struct SKAnimator* animator, void* data, struct SKAnimationEvent* event) {
    struct Player* player = (struct Player*)data;

    if (PLAYER_IS_ATTACK_EVENT(event->id)) {
        playerStartAttack(player, &mAttacks[PLAYER_ATTACK_EVENT_GET_SOURCE(event->id)]);
    } else if (event->id == PLAYER_ATTACK_END_ID) {
        playerEndAttack(player);
    }
}

void playerStateWalk(struct Player* player, struct PlayerInput* input);

void playerInit(struct Player* player, unsigned team, struct Vector2* at) {
    player->team.entityType = TeamEntityTypePlayer;
    player->team.teamNumber = team;
    transformInitIdentity(&player->transform);
    player->transform.position.x = at->x;
    player->transform.position.z = at->y;
    player->attackCollider = 0;
    player->attackInfo = 0;

    player->velocity = gZeroVec;
    player->rightDir = gRight2;

    player->collider = dynamicSceneNewEntry(
        &gPlayerCollider.shapeCommon,
        player,
        at,
        teamEntityCorrectOverlap,
        DynamicSceneEntryHasTeam,
        CollisionLayersTangible | CollisionLayersBase | COLLISION_LAYER_FOR_TEAM(team)
    );

    player->state = playerStateWalk;

    recallCircleInit(&player->recallCircle);
    
    skInitObject(
        &player->armature, 
        doglow_DogLow_mesh, 
        DOGLOW_DEFAULT_BONES_COUNT, 
        CALC_ROM_POINTER(character_animations, doglow_default_bones),
        CALC_RAM_POINTER(doglow_bone_parent, gCharacterSegment)
    );

    skAnimatorInit(&player->animator, DOGLOW_DEFAULT_BONES_COUNT, playerAnimationEvent, player);
    skAnimatorRunClip(&player->animator, &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_IDLE_INDEX], SKAnimatorFlagsLoop);
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

    if (input->actionFlags & PlayerInputActionsCommandRecall) {
        struct Vector3 recallPos3;
        transformPoint(&player->transform, &gRecallOffset, &recallPos3);
        struct Vector2 recallPos2;
        recallPos2.x = recallPos3.x;
        recallPos2.y = recallPos3.z;
        recallCircleActivate(&player->recallCircle, &recallPos2, player->team.teamNumber);
    } else {
        recallCircleDisable(&player->recallCircle);
    }

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

    player->collider->center.x = player->transform.position.x;
    player->collider->center.y = player->transform.position.z;

    struct LevelBase *lastBase = gPlayerAtBase[player->team.teamNumber];

    if (lastBase) {
        baseCommandMenuShowOpenCommand(&gCurrentLevel.baseCommandMenu[player->team.teamNumber], lastBase);

        if (playerInputGetDown(input, PlayerInputActionsCommandOpenBaseMenu)) {
            baseCommandMenuShow(&gCurrentLevel.baseCommandMenu[player->team.teamNumber], lastBase);
        }
    } else {
        baseCommandMenuHideOpenCommand(&gCurrentLevel.baseCommandMenu[player->team.teamNumber]);
    }

    gPlayerAtBase[player->team.teamNumber] = 0;

    skAnimatorUpdate(&player->animator, player->armature.boneTransforms, 1.0f);

    if (player->attackCollider && player->attackInfo) {
        playerCalculateAttackLocation(player, player->attackInfo, &player->collider->center);
    }

    if (lastBase && 
        lastBase->state == LevelBaseStateSpawning && 
        (controllersGetControllerData(player->team.teamNumber)->button & Z_TRIG) != 0) {
        levelBaseStartUpgrade(lastBase, LevelBaseStateUpgradingCapacity);
    }

    if (input->actionFlags & PlayerInputActionsCommandAttack) {
        levelSceneIssueMinionCommand(&gCurrentLevel, player->team.teamNumber, MinionCommandAttack);
    } else if (input->actionFlags & PlayerInputActionsCommandDefend) {
        levelSceneIssueMinionCommand(&gCurrentLevel, player->team.teamNumber, MinionCommandDefend);
    }
}

void playerRender(struct Player* player, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&player->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    skRenderObject(&player->armature, renderState);
    gSPPopMatrix(renderState->dl++, 1);

    recallCircleRender(&player->recallCircle, renderState, player->team.teamNumber);
}