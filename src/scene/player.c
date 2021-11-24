
#include "player.h"
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
#include "events.h"
#include "collision/staticscene.h"
#include <stdbool.h>

#define PLAYER_MAX_HP                              6.0f
#define PLAYER_MIN_RESPAWN_TIME                    5.0f
#define PLAYER_RESPAWN_PER_BASE                    2.0f
#define PLAYER_INVINCIBILITY_TIME                  0.5f
#define INVINCIBLE_JUMP_HEIGHT                     1.0f

struct CollisionCircle gPlayerCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * 0.5f,
};

struct Vector3 gRecallOffset = {0.0f, 0.0f, -4.0 * SCENE_SCALE};

#define PLAYER_AIR_SPEED_SCALAR     0.8f
#define PLAYER_STOP_ACCELERATION    50.0f
#define PLAYER_AIR_ACCELERATION     10.0f

#define PLAYER_JUMP_ACCEL           (-GRAVITY * 0.5f)

#define PLAYER_JUMP_IMPULSE         18.0f

#define PLAYER_AIR_MAX_ROTATE_SEC   (90.0f * (M_PI / 180.0f))
#define PLAYER_MAX_ROTATE_SEC       (300.0f * (M_PI / 180.0f))
#define PLAYER_ATTACK_MAX_ROTATE_SEC (180.0f * (M_PI / 180.0f))
#define PLAYER_JUMP_ATTACK_HOVER_TIME       0.1f
#define PLAYER_JUMP_ATTACK_FALL_VELOICTY    -20.0f
#define PLAYER_JUMP_ATTCK_DELAY             0.5f


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
        soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.attackSounds), 0);
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
void playerStateJump(struct Player* player, struct PlayerInput* input);
void playerStateJumpAttackStart(struct Player* player, struct PlayerInput* input);
void playerStateDelay(struct Player* player, struct PlayerInput* input);

void playerEnterWalkState(struct Player* player) {
    player->state = playerStateWalk;
    playerEndAttack(player);
    player->attackInfo = 0;
}

void playerEnterAttackState(struct Player* player, struct PlayerAttackInfo* attackInfo) {
    skAnimatorRunClip(&player->animator, attackInfo->animation, 0);
    player->attackInfo = attackInfo;
    player->state = playerStateAttack;
    player->animationSpeed = 1.0f;
}

void playerEnterDeadState(struct Player* player) {
    playerEndAttack(player);
    soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.deathSounds), 0);
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationDie), 0);
    player->state = playerStateDead;
    player->stateTimer = PLAYER_MIN_RESPAWN_TIME + (player->controlledBases - 1) * PLAYER_RESPAWN_PER_BASE;
    player->collider->collisionLayers = 0;
}

void playerEnterJumpState(struct Player* player) {
    player->velocity.y = PLAYER_JUMP_IMPULSE;
    player->state = playerStateJump;
    player->animationSpeed = 1.0f;
    soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.jumpSounds), 0);
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationJump), 0);
}

void playerEnterJumpAttackState(struct Player* player) {
    player->state = playerStateJumpAttackStart;
    player->stateTimer = PLAYER_JUMP_ATTACK_HOVER_TIME;
    player->velocity = gZeroVec;
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationJumpAttack), 0);
}

void playerEnterDelayState(struct Player* player, float duration) {
    player->state = playerStateDelay;
    player->stateTimer = duration;
}

void playerUpdatePosition(struct Player* player) {
    player->velocity.y += GRAVITY * gTimeDelta;
    vector3AddScaled(&player->transform.position, &player->velocity, gTimeDelta * SCENE_SCALE, &player->transform.position);

    if (player->transform.position.y < FLOOR_HEIGHT) {
        player->transform.position.y = FLOOR_HEIGHT;
        player->velocity.y = 0.0f;
    }
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

    damageHandlerUpdate(&player->damageHandler);

    if (player->damageHandler.hp <= 0.0f) {
        gLastDeathTime = gTimePassed;
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

void playerCorrectOverlap(struct DynamicSceneOverlap* overlap) {
    struct Player* player = (struct Player*)overlap->thisEntry->data;

    struct Vector2 normVelocity;
    struct Vector2 vel2D;
    vel2D.x = player->velocity.x;
    vel2D.y = player->velocity.z;
    vector2Scale(&overlap->shapeOverlap.normal, vector2Dot(&vel2D, &overlap->shapeOverlap.normal), &normVelocity);
    vector2Sub(&vel2D, &normVelocity, &vel2D);
    player->velocity.x = vel2D.x;
    player->velocity.z = vel2D.y;

    teamEntityCorrectOverlap(overlap);
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
    damageHandlerInit(&player->damageHandler, PLAYER_MAX_HP);
    player->walkSoundEffect = SOUND_ID_NONE;
    player->idleSoundEffect = SOUND_ID_NONE;
    player->animationSpeed = 1.0f;
    player->controlledBases = 0;

    player->velocity = gZeroVec;
    player->rightDir = gRight2;

    player->collider = dynamicSceneNewEntry(
        &gPlayerCollider.shapeCommon,
        player,
        at,
        playerCorrectOverlap,
        DynamicSceneEntryHasTeam,
        CollisionLayersTangible | CollisionLayersBase | COLLISION_LAYER_FOR_TEAM(team)
    );

    player->state = playerStateWalk;

    recallCircleInit(&player->recallCircle);
    
    skArmatureInit(
        &player->armature, 
        gTeamFactions[player->team.teamNumber]->playerMesh, 
        gTeamFactions[player->team.teamNumber]->playerBoneCount, 
        CALC_ROM_POINTER(character_animations, gTeamFactions[player->team.teamNumber]->playerDefaultPose),
        gTeamFactions[player->team.teamNumber]->playerBoneParent
    );

    skAnimatorInit(&player->animator, gTeamFactions[player->team.teamNumber]->playerBoneCount, playerAnimationEvent, player);
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationIdle), SKAnimatorFlagsLoop);
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

void playerUpdateAttack(struct Player* player) {
    if (player->attackCollider && player->attackInfo) {
        struct Vector3 attackPosition;
        playerCalculateAttackLocation(player, player->attackInfo, &attackPosition);
        punchTrailUpdate(&player->punchTrail, &attackPosition);
        dynamicEntrySetPos3D(player->attackCollider, &attackPosition);
    }
}

void playerStateDelay(struct Player* player, struct PlayerInput* input) {
    player->stateTimer -= gTimeDelta;
    
    if (player->stateTimer <= 0.0f) {
        playerEnterWalkState(player);
    }
}

void playerStateJumpAttack(struct Player* player, struct PlayerInput* input) {
    player->velocity.y = PLAYER_JUMP_ATTACK_FALL_VELOICTY;

    if (player->transform.position.y <= 0.0f && player->velocity.y <= 0.0f) {
        skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationJumpAttackLanding), 0);
        playerEndAttack(player);
        playerEnterDelayState(player, PLAYER_JUMP_ATTCK_DELAY);
    }

    playerUpdateAttack(player);
    
    playerUpdateOther(player, input);
}

void playerStateJumpAttackStart(struct Player* player, struct PlayerInput* input) {
    if (player->stateTimer <= 0.0f) {
        player->attackInfo = factionGetAttack(player->team.teamNumber, PlayerAttackJumpAttack);
        playerStartAttack(player);
        player->state = playerStateJumpAttack;
    } else {
        player->stateTimer -= gTimeDelta;
    }
}

void playerStateFreefall(struct Player* player, struct PlayerInput* input) {
    float moveSpeed = gTeamFactions[player->team.teamNumber]->moveSpeed;

    playerRotateTowardsInput(player, input, PLAYER_AIR_MAX_ROTATE_SEC);
    playerAccelerateTowards(
        player, 
        &input->targetWorldDirection, 
        PLAYER_AIR_SPEED_SCALAR * moveSpeed, 
        PLAYER_AIR_ACCELERATION, 
        PLAYER_AIR_ACCELERATION
    );

    int wasGoingUp = player->velocity.y > 0.0f;


    if (playerInputGetDown(input, PlayerInputActionsAttack)) {
        playerEnterJumpAttackState(player);
        return;
    }

    if (player->transform.position.y <= 0.0f && player->velocity.y <= 0.0f) {
        playerEnterWalkState(player);
    }

    playerUpdateOther(player, input);

    if (player->velocity.y <= 0.0f && wasGoingUp) {
        skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationFall), 0);
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

void playerStateAttack(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_ATTACK_MAX_ROTATE_SEC);
    playerAccelerateTowards(player, &gZeroVec, 0.0f, PLAYER_STOP_ACCELERATION, PLAYER_STOP_ACCELERATION);

    if (playerInputGetDown(input, PlayerInputActionsAttack)) {
        if ((player->flags & (PlayerFlagsInAttackWindow | PlayerFlagsAttackEarly)) == PlayerFlagsInAttackWindow) {
            playerEnterAttackState(player, factionGetAttack(player->team.teamNumber, player->attackInfo->chainedTo));
        } else {
            player->flags |= PlayerFlagsAttackEarly;
        }
    }

    playerUpdateAttack(player);

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
            damageHandlerInit(&player->damageHandler, PLAYER_MAX_HP);
            playerEnterWalkState(player);
        }
    }

    player->stateTimer -= gTimeDelta;
}

void playerStateWalk(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_MAX_ROTATE_SEC);

    struct Vector3 forwardDir;
    quatMultVector(&player->transform.rotation, &gForward, &forwardDir);
    vector3Project(&player->velocity, &forwardDir, &player->velocity);

    float accelerateScale = vector3Dot(&forwardDir, &input->targetWorldDirection);
    float inputScale = vector3MagSqrd(&input->targetWorldDirection);

    if (accelerateScale > 0.0f || inputScale < 0.03f) {
        accelerateScale = 0.0f;
    }

    vector3Scale(&forwardDir, &forwardDir, accelerateScale * sqrtf(inputScale));

    float moveSpeed = gTeamFactions[player->team.teamNumber]->moveSpeed;

    playerAccelerateTowards(player, &forwardDir, moveSpeed, PLAYER_MOVE_ACCELERATION, PLAYER_STOP_ACCELERATION);

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

    float speed = sqrtf(vector3MagSqrd(&player->velocity));

    player->animationSpeed = speed / moveSpeed;

    int isMoving = speed > 0.01f;

    if (input->actionFlags & PlayerInputActionsJump) {
        playerEnterJumpState(player);
        isMoving = 0;
    } else if (playerInputGetDown(input, PlayerInputActionsAttack)) {
        playerEnterAttackState(player, factionGetAttack(player->team.teamNumber, PlayerAttackPunch));
        isMoving = 0;
    } else {
        if (isMoving) {
            if (player->animator.currentAnimation != factionGetAnimation(player->team.teamNumber, PlayerAnimationWalk)) {
                skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationWalk), SKAnimatorFlagsLoop);
            }
        } else {
            player->animationSpeed = 1.0f;
            if (player->animator.currentAnimation != factionGetAnimation(player->team.teamNumber, PlayerAnimationIdle)) {
                skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationIdle), SKAnimatorFlagsLoop);
            }
        }
    }

    int hasWalkingSound = player->walkSoundEffect != SOUND_ID_NONE;

    if (isMoving != hasWalkingSound) {
        if (isMoving) {
            player->walkSoundEffect = soundPlayerPlay(gTeamFactions[player->playerIndex]->playerSounds.walkSound, SoundPlayerFlagsLoop);
            soundPlayerSetVolume(player->walkSoundEffect, 0.25f);
        } else {
            soundPlayerStop(&player->walkSoundEffect);
        }
    }

    // int hasIdleSound = player->idleSoundEffect != SOUND_ID_NONE;

    // if (isMoving == hasIdleSound) {
    //     if (isMoving) {
    //         soundPlayerStop(&player->idleSoundEffect);
    //     } else {
    //         player->idleSoundEffect = soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.idleSounds), SoundPlayerFlagsLoop);
    //     }
    // }

    playerUpdateOther(player, input);
}

void playerUpdate(struct Player* player, struct PlayerInput* input) {
    player->state(player, input);
    skAnimatorUpdate(&player->animator, player->armature.boneTransforms, player->animationSpeed);
    dynamicEntrySetPos3D(player->collider, &player->transform.position);
    struct Vector2 vel2D;
    vel2D.x = player->velocity.x;
    vel2D.y = player->velocity.z;
    staticSceneConstrainToBoundaries(&gCurrentLevel.definition->staticScene, &player->collider->center, &vel2D, gPlayerCollider.radius);
    player->collider->flags |= DynamicSceneEntryDirtyBox;
    player->transform.position.x = player->collider->center.x;
    player->transform.position.z = player->collider->center.y;
    player->velocity.x = vel2D.x;
    player->velocity.z = vel2D.y;
}

void playerRender(struct Player* player, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    transformToMatrixL(&player->transform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    int isDamageFlash = damageHandlerIsFlashing(&player->damageHandler);

    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, gTeamPalleteTexture[isDamageFlash ? DAMAGE_PALLETE_INDEX : player->team.teamNumber]);
    skRenderObject(&player->armature, renderState);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

    recallCircleRender(&player->recallCircle, renderState, player->team.teamNumber);
    if (player->attackCollider) {
        punchTrailRender(&player->punchTrail, renderState);
    }
}

void playerApplyDamage(struct Player* player, float amount) {
    if (player->transform.position.y < INVINCIBLE_JUMP_HEIGHT) {
        if (damageHandlerApplyDamage(&player->damageHandler, amount, PLAYER_INVINCIBILITY_TIME)) {
            soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.damageSounds), 0);
        }
    }
}

int playerIsAlive(struct Player* player) {
    return player->damageHandler.hp > 0 && player->state != playerStateDead;
}