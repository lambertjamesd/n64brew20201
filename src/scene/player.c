
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
#include "sk64/skelatool_clip.h"
#include "audio/soundplayer.h"
#include "audio/clips.h"
#include "math/mathf.h"
#include "team_data.h"

#define PLAYER_ATTACK_START_ID                     0x0
#define PLAYER_ATTACK_END_ID                       0x1
#define PLAYER_ATTACK_WINDOW_ID                    0x2
#define PLAYER_MAX_HP                              8.0f
#define PLAYER_RESPAWN_TIME                        5.0f

struct SKAnimationEvent gAttack001Events[] = {
    {8, PLAYER_ATTACK_START_ID},
    {10, PLAYER_ATTACK_WINDOW_ID},
    {10, PLAYER_ATTACK_END_ID},
};

struct SKAnimationEvent gAttack002Events[] = {
    {6, PLAYER_ATTACK_START_ID},
    {10, PLAYER_ATTACK_END_ID},
};

unsigned short gJumpClipIds[] = {
    SOUNDS_DOG_JUMP_GRUNT_1,
    SOUNDS_DOG_JUMP_GRUNT_2,
    SOUNDS_DOG_JUMP_GRUNT_3,
    SOUNDS_DOG_JUMP_GRUNT_4,
    SOUNDS_DOG_JUMP_GRUNT_5,
    SOUNDS_DOG_JUMP_GRUNT_6,
    SOUNDS_DOG_JUMP_GRUNT_7,
};

#define ATTACK_001_EVENT_COUNT      (sizeof(gAttack001Events)/sizeof(*gAttack001Events))
#define ATTACK_002_EVENT_COUNT      (sizeof(gAttack001Events)/sizeof(*gAttack002Events))

struct PlayerAttackInfo gPlayerAttacks[] = {
    {
        DOGLOW_ARM1_BONE, 
        1,
        DOGLOW_DOGLOW_ARMATURE_001_PUNCH_001_INDEX,
        2.0f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
    },
    {
        DOGLOW_ARM2_BONE, 
        0,
        DOGLOW_DOGLOW_ARMATURE_001_PUNCH_002_INDEX,
        3.0f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
    },
};

struct CollisionCircle gPlayerCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * 0.5f,
};

struct Vector3 gRecallOffset = {0.0f, 0.0f, -4.0 * SCENE_SCALE};

#include "../data/models/doglow/geometry_animdef.inc.h"

#define PLAYER_AIR_SPEED            (PLAYER_MOVE_SPEED * 0.8f)
#define PLAYER_STOP_ACCELERATION    50.0f
#define PLAYER_AIR_ACCELERATION     10.0f

#define PLAYER_JUMP_ACCEL           (-GRAVITY * 0.5f)

#define PLAYER_JUMP_IMPULSE         12.0f

#define PLAYER_AIR_MAX_ROTATE_SEC   (90.0f * (M_PI / 180.0f))
#define PLAYER_MAX_ROTATE_SEC       (500.0f * (M_PI / 180.0f))
#define PLAYER_ATTACK_MAX_ROTATE_SEC (180.0f * (M_PI / 180.0f))

void playerCalculateAttackLocation(struct Player* player, struct PlayerAttackInfo* attackInfo, struct Vector3* output) {
    skCalculateBonePosition(&player->armature, attackInfo->boneIndex, &attackInfo->localPosition, output);
    transformPoint(&player->transform, output, output);
}

void playerAttackOverlap(struct DynamicSceneOverlap* overlap) {
    struct Player* player = (struct Player*)overlap->thisEntry->data;

    if ((overlap->otherEntry->flags & DynamicSceneEntryHasTeam) != 0 && player->attackInfo) {
        struct TeamEntity* entityB = (struct TeamEntity*)overlap->otherEntry->data;
        teamEntityApplyDamage(entityB, player->attackInfo->damage);
    }
}

void playerStartAttack(struct Player* player) {
    if (!player->attackCollider) {
        struct Vector3 position3D;
        struct Vector2 position;
        playerCalculateAttackLocation(player, player->attackInfo, &position3D);
        position.x = position3D.x;
        position.y = position3D.z;
        player->attackCollider = dynamicSceneNewEntry(
            &player->attackInfo->collisionCircle.shapeCommon, 
            player,
            &position,
            playerAttackOverlap,
            DynamicSceneEntryIsTrigger,
            CollisionLayersAllTeams ^ COLLISION_LAYER_FOR_TEAM(player->team.teamNumber)
        );
        punchTrailInit(&player->punchTrail, &position3D, player->attackInfo->collisionCircle.radius / SCENE_SCALE);
    }
}

void playerEndAttack(struct Player* player) {
    if (player->attackCollider) {
        dynamicSceneDeleteEntry(player->attackCollider);
        player->attackCollider = 0;
    }
}

void playerStateAttack(struct Player* player, struct PlayerInput* input);
void playerStateWalk(struct Player* player, struct PlayerInput* input);
void playerStateAttack(struct Player* player, struct PlayerInput* input);
void playerStateDead(struct Player* player, struct PlayerInput* input);

void playerEnterWalkState(struct Player* player) {
    player->state = playerStateWalk;
    player->attackInfo = 0;
    // skAnimatorRunClip(&player->animator, &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_IDLE_INDEX], SKAnimatorFlagsLoop);
}

void playerEnterAttackState(struct Player* player, struct PlayerAttackInfo* attackInfo) {
    skAnimatorRunClip(&player->animator, &doglow_animations[attackInfo->animationId], 0);
    player->attackInfo = attackInfo;
    player->state = playerStateAttack;
}

void playerEnterDeadState(struct Player* player) {
    playerEndAttack(player);
    skAnimatorRunClip(&player->animator, &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_DIE_INDEX], 0);
    player->state = playerStateDead;
    player->stateTimer = PLAYER_RESPAWN_TIME;
    player->collider->collisionLayers = 0;
}

void playerUpdatePosition(struct Player* player) {
    player->velocity.y += GRAVITY * gTimeDelta;
    vector3AddScaled(&player->transform.position, &player->velocity, gTimeDelta * SCENE_SCALE, &player->transform.position);

    if (player->transform.position.y < FLOOR_HEIGHT) {
        player->transform.position.y = FLOOR_HEIGHT;
        player->velocity.y = 0.0f;
    }

    player->collider->center.x = player->transform.position.x;
    player->collider->center.y = player->transform.position.z;
}

void playerUpdateLastBase(struct Player* player, struct PlayerInput* input) {
    struct LevelBase *lastBase = gPlayerAtBase[player->playerIndex];

    if (lastBase) {
        baseCommandMenuShowOpenCommand(&gCurrentLevel.baseCommandMenu[player->playerIndex], lastBase);

        if (playerInputGetDown(input, PlayerInputActionsCommandOpenBaseMenu)) {
            baseCommandMenuShow(&gCurrentLevel.baseCommandMenu[player->playerIndex], lastBase);
        }
    } else {
        baseCommandMenuHideOpenCommand(&gCurrentLevel.baseCommandMenu[player->playerIndex]);
    }

    gPlayerAtBase[player->playerIndex] = 0;
}

void playerCheckCommands(struct Player* player, struct PlayerInput* input) {
    if (input->actionFlags & PlayerInputActionsCommandAttack) {
        levelSceneIssueMinionCommand(&gCurrentLevel, player->playerIndex, MinionCommandAttack);
    } else if (input->actionFlags & PlayerInputActionsCommandDefend) {
        levelSceneIssueMinionCommand(&gCurrentLevel, player->playerIndex, MinionCommandDefend);
    }
}

void playerUpdateOther(struct Player* player, struct PlayerInput* input) {
    playerUpdatePosition(player);
    playerUpdateLastBase(player, input);
    playerCheckCommands(player, input);

    if (player->hp <= 0.0f) {
        playerEnterDeadState(player);
    }
}

void playerAnimationEvent(struct SKAnimator* animator, void* data, struct SKAnimationEvent* event) {
    struct Player* player = (struct Player*)data;

    switch (event->id) {
        case PLAYER_ATTACK_START_ID:
            playerStartAttack(player);
            break;
        case PLAYER_ATTACK_END_ID:
            playerEndAttack(player);
            break;
        case PLAYER_ATTACK_WINDOW_ID:
            player->flags |= PlayerFlagsInAttackWindow;
            break;
        case SK_ANIMATION_EVENT_START:
            player->flags &= ~(PlayerFlagsInAttackWindow | PlayerFlagsAttackEarly);
            break;
        case SK_ANIMATION_EVENT_END:
            if (player->state == playerStateAttack) {
                playerEnterWalkState(player);
            }
            break;
    }
}

void playerInit(struct Player* player, unsigned playerIndex, unsigned team, struct Vector2* at) {
    player->team.entityType = TeamEntityTypePlayer;
    player->team.teamNumber = team;
    transformInitIdentity(&player->transform);
    player->transform.position.x = at->x;
    player->transform.position.z = at->y;
    player->attackCollider = 0;
    player->attackInfo = 0;
    player->playerIndex = playerIndex;
    player->flags = 0;
    player->hp = PLAYER_MAX_HP;
    player->walkSoundEffect = SOUND_ID_NONE;

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
        doglow_bone_parent
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

    playerUpdateOther(player, input);
}

void playerStateJump(struct Player* player, struct PlayerInput* input) {
    if (input->actionFlags & PlayerInputActionsJump) {
        player->velocity.y += PLAYER_JUMP_ACCEL * gTimeDelta;
        playerStateFreefall(player, input);
    } else {
        player->state = playerStateFreefall;
    }
}

void playerStateAttack(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_ATTACK_MAX_ROTATE_SEC);
    playerAccelerateTowards(player, &gZeroVec, 0.0f, PLAYER_STOP_ACCELERATION, PLAYER_STOP_ACCELERATION);

    if (playerInputGetDown(input, PlayerInputActionsAttack)) {
        if ((player->flags & (PlayerFlagsInAttackWindow | PlayerFlagsAttackEarly)) == PlayerFlagsInAttackWindow) {
            playerEnterAttackState(player, &gPlayerAttacks[player->attackInfo->chainedTo]);
        } else {
            player->flags |= PlayerFlagsAttackEarly;
        }
    }

    if (player->attackCollider && player->attackInfo) {
        struct Vector3 attackPosition;
        playerCalculateAttackLocation(player, player->attackInfo, &attackPosition);
        punchTrailUpdate(&player->punchTrail, &attackPosition);
        player->collider->center.x = attackPosition.x;
        player->collider->center.y = attackPosition.z;
    }

    playerUpdateOther(player, input);
}

void playerStateDead(struct Player* player, struct PlayerInput* input) {
    playerAccelerateTowards(player, &gZeroVec, 0.0f, PLAYER_MOVE_ACCELERATION, PLAYER_STOP_ACCELERATION);
    playerUpdatePosition(player);

    if (player->stateTimer <= 0.0f) {
        struct Vector3* respawnPoint = levelSceneFindRespawnPoint(&gCurrentLevel, &player->transform.position, player->team.teamNumber);
        if (respawnPoint) {
            player->collider->collisionLayers = CollisionLayersTangible | CollisionLayersBase | COLLISION_LAYER_FOR_TEAM(player->team.teamNumber);
            player->transform.position = *respawnPoint;
            player->hp = PLAYER_MAX_HP;
            playerEnterWalkState(player);
        }
    }

    player->stateTimer -= gTimeDelta;
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
        recallCircleActivate(&player->recallCircle, &recallPos2, player);
    } else {
        recallCircleDisable(&player->recallCircle);
    }

    int isMoving = vector3MagSqrd(&player->velocity) > 0.0001f;

    if (input->actionFlags & PlayerInputActionsJump) {
        player->velocity.y = PLAYER_JUMP_IMPULSE;
        player->state = playerStateJump;
        soundPlayerPlay(gJumpClipIds[randomInRange(0, sizeof(gJumpClipIds)/sizeof(&gJumpClipIds))], 0);
        isMoving = 0;
    }

    if (playerInputGetDown(input, PlayerInputActionsAttack)) {
        playerEnterAttackState(player, &gPlayerAttacks[0]);
        isMoving = 0;
    }

    int hasWalkingSound = player->walkSoundEffect != SOUND_ID_NONE;

    if (isMoving != hasWalkingSound) {
        if (isMoving) {
            player->walkSoundEffect = soundPlayerPlay(SOUNDS_DOG_WALKING_LOOP_1, SoundPlayerFlagsLoop);
        } else {
            soundPlayerStop(&player->walkSoundEffect);
        }
    }

    playerUpdateOther(player, input);
}

void playerUpdate(struct Player* player, struct PlayerInput* input) {
    player->state(player, input);
    skAnimatorUpdate(&player->animator, player->armature.boneTransforms, 1.0f);
}

void playerRender(struct Player* player, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&player->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, gTeamPalleteTexture[player->team.teamNumber]);
    skRenderObject(&player->armature, renderState);
    gSPPopMatrix(renderState->dl++, 1);

    recallCircleRender(&player->recallCircle, renderState, player->team.teamNumber);
    if (player->attackCollider) {
        punchTrailRender(&player->punchTrail, renderState);
    }
}