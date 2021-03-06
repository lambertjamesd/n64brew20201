
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
#include "graphics/spritefont.h"
#include "menu/kickflipfont.h"
#include <stdbool.h>
#include "math.h"

#define PLAYER_MIN_RESPAWN_TIME                    5.0f
#define PLAYER_RESPAWN_PER_BASE                    2.0f
#define PLAYER_INVINCIBILITY_TIME                  0.40f
#define INVINCIBLE_JUMP_HEIGHT                     1.0f
#define CHARGE_SPIN_ATTACK_TIME                    0.75f

#define SPIN_ATTACK_MOVE_SPEED                     (PLAYER_BASE_MOVE_SPEED * 2.0f)
#define SPIN_ATTACK_ACCELL                         (40.0f)

struct CollisionCircle gPlayerCollider = {
    {CollisionShapeTypeCircle},
    PLAYER_COLLIDER_RADIOUS,
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
#define PLAYER_JUMP_ATTCK_DELAY             0.35f

#define PLAYER_FOOTSTEP_LEN          1.f
#define PLAYER_FOOTSTEP_TIMER_DECREAMENT    0.1f

void playerEnterAttackState(struct Player* player, struct PlayerAttackInfo* attackInfo);

int aiAttackPriority(struct TeamEntity* target) {
    return target->entityType;
}

void playerCalculateAttackLocation(struct Player* player, struct PlayerAttackInfo* attackInfo, struct Vector3* output) {
    skCalculateBonePosition(&player->armature, attackInfo->boneIndex, &attackInfo->localPosition, output);
    transformPoint(&player->transform, output, output);
}

void playerAttackOverlap(struct DynamicSceneOverlap* overlap) {
    struct Player* player = (struct Player*)overlap->thisEntry->data;

    if ((overlap->otherEntry->flags & DynamicSceneEntryHasTeam) != 0 && player->attackInfo) {
        struct TeamEntity* entityB = (struct TeamEntity*)overlap->otherEntry->data;
        teamEntityApplyDamage(entityB, player->attackInfo->damage, &player->transform.position, player->attackInfo->knockback);
        soundPlayerPlay(SOUNDS_PUNCHIMPACT, 1.0f, gTeamFactions[player->team.teamNumber]->soundPitchScalar, SoundPlayerPriorityPlayer, 0, &player->transform.position);
    }
}

void playerStartAttack(struct Player* player) {
    if (!player->attackCollider) {
        struct Vector3 position3D;
        struct Vector2 position;
        soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.attackSounds), 1.0f, gTeamFactions[player->team.teamNumber]->soundPitchScalar, SoundPlayerPriorityPlayer, 0, &player->transform.position);
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

        if (player->flags & PlayerFlagsAttackSuccess) {
            playerEnterAttackState(player, factionGetAttack(player->team.teamNumber, player->attackInfo->chainedTo));
        }
    }
}

void playerStateAttack(struct Player* player, struct PlayerInput* input);
void playerStateWalk(struct Player* player, struct PlayerInput* input);
void playerStateAttackCharge(struct Player* player, struct PlayerInput* input);
void playerStateDead(struct Player* player, struct PlayerInput* input);
void playerStateJump(struct Player* player, struct PlayerInput* input);
void playerStateJumpAttackStart(struct Player* player, struct PlayerInput* input);
void playerStateDelay(struct Player* player, struct PlayerInput* input);
void playerStateSpawn(struct Player* player, struct PlayerInput* input);

void playerEnterWalkState(struct Player* player) {
    player->state = playerStateWalk;
    playerEndAttack(player);
    player->attackInfo = 0;
}

void playerEnterSpawnState(struct Player* player) {
    player->state = playerStateSpawn;
    soundPlayerPlay(SOUNDS_SPAWN, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, &player->transform.position);
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationSpawn), 0);
    player->animationSpeed = 1.0f;
    player->attackInfo = 0;
}

void playerEnterAttackState(struct Player* player, struct PlayerAttackInfo* attackInfo) {
    skAnimatorRunClip(&player->animator, attackInfo->animation, 0);
    player->attackInfo = attackInfo;
    player->state = playerStateAttack;
    player->flags &= ~PlayerFlagsAttackSuccess;
    player->animationSpeed = 1.0f;
}

void playerEnterAttackChargeState(struct Player* player) {
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationCrouch), 0);
    player->stateTimer = CHARGE_SPIN_ATTACK_TIME;
    player->state = playerStateAttackCharge;
    player->animationSpeed = 1.0f;
}

void playerEnterDeadState(struct Player* player) {
    // any minions that were following the player when they died
    // should start attacking nearest target
    levelSceneIssueMinionCommand(&gCurrentLevel, player->playerIndex, MinionCommandAttack);
    playerEndAttack(player);
    soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.deathSounds), 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, &player->transform.position);
    //soundPlayerPlay(SOUNDS_DEATHSFX, 0);
    skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationDie), 0);
    player->state = playerStateDead;
    player->stateTimer = PLAYER_MIN_RESPAWN_TIME + (player->controlledBases - 1) * PLAYER_RESPAWN_PER_BASE;
    player->collider->collisionLayers = 0;
    player->animationSpeed = 1.0f;
    player->attackInfo = 0;
    player->tilt = 0;
}

void playerEnterJumpState(struct Player* player) {
    player->velocity.y = PLAYER_JUMP_IMPULSE;
    player->state = playerStateJump;
    player->animationSpeed = 1.0f;
    player->flags &= ~PlayerFlagsKnockedBack;
    soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.jumpSounds), 1.0f, gTeamFactions[player->team.teamNumber]->soundPitchScalar, SoundPlayerPriorityPlayer, 0, &player->transform.position);
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
            if (player->state == playerStateAttack || player->state == playerStateSpawn) {
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

    float velocityDot = vector2Dot(&vel2D, &overlap->shapeOverlap.normal);

    if (velocityDot * overlap->shapeOverlap.depth > 0.0f) {
        vector2Scale(&overlap->shapeOverlap.normal, velocityDot, &normVelocity);
        vector2Sub(&vel2D, &normVelocity, &vel2D);
        player->velocity.x = vel2D.x;
        player->velocity.z = vel2D.y;
    }

    if (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) {
        struct TeamEntity* otherEntity = (struct TeamEntity*)overlap->otherEntry->data;

        if (otherEntity->teamNumber != player->team.teamNumber &&
            (player->touchedBy == 0 || aiAttackPriority(player->touchedBy) < aiAttackPriority(otherEntity))) {
            player->touchedBy = otherEntity;
        }
    }


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
    damageHandlerInit(&player->damageHandler, gTeamFactions[player->team.teamNumber]->maxHP);
    player->walkSoundEffect = SoundIDNone;
    player->idleSoundEffect = SoundIDNone;
    player->animationSpeed = 1.0f;
    player->footstepTimer = -1.f;
    player->controlledBases = 0;
    player->touchedBy = 0;
    player->aiTarget = 0;
    player->tilt = 0;

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

    int closestBase = aiPlannerFindNearestBaseToPoint(&gCurrentLevel, &player->transform.position, player->team.teamNumber, EnemyTeam, 0);

    if (closestBase >= 0) {
        struct Vector3 towardsBase;
        vector3Sub(&gCurrentLevel.bases[closestBase].position, &player->transform.position, &towardsBase);
        quatLook(&towardsBase, &gUp, &player->transform.rotation);
    }

    playerEnterSpawnState(player);
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

void playerStateSpawn(struct Player* player, struct PlayerInput* input) {
    // the animation event handler takes care of transitioning into 
    // the walk state
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


    if ((player->flags & PlayerFlagsKnockedBack) == 0 && playerInputGetDown(input, PlayerInputActionsAttack)) {
        playerEnterJumpAttackState(player);
        return;
    }

    if (player->transform.position.y <= 0.0f && player->velocity.y <= 0.0f) {
        player->flags &= ~PlayerFlagsKnockedBack;
        playerEnterWalkState(player);
    }

    player->tilt = mathfLerp(player->tilt, 0.0f, 0.1f);

    if (player->velocity.y <= 0.0f && wasGoingUp) {
        skAnimatorRunClip(&player->animator, factionGetAnimation(player->team.teamNumber, PlayerAnimationFall), 0);
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

void playerStateAttackCharge(struct Player* player, struct PlayerInput* input) {
    playerAccelerateTowards(player, &gZeroVec, 0.0f, PLAYER_STOP_ACCELERATION, PLAYER_STOP_ACCELERATION);

    if (!(input->actionFlags & PlayerInputActionsAttack)) {
        if (player->stateTimer > 0.0f) {
            playerEnterAttackState(player, factionGetAttack(player->team.teamNumber, PlayerAttackPunch));
        } else {
            vector3Scale(&input->targetWorldDirection, &player->velocity, SPIN_ATTACK_MOVE_SPEED * 0.25f);
            playerEnterAttackState(player, factionGetAttack(player->team.teamNumber, PlayerAttackSpinAttack));
        }
    }

    player->tilt = mathfLerp(player->tilt, 0.0f, 0.1f);

    if (player->stateTimer > 0.0f) {
        player->stateTimer -= gTimeDelta;
    }

    playerUpdateOther(player, input);
}

void playerStateAttack(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_ATTACK_MAX_ROTATE_SEC);
    playerAccelerateTowards(
        player, 
        &input->targetWorldDirection, 
        (player->attackInfo->flags & PlayerAttackInfoFlagsCanMove) ? SPIN_ATTACK_MOVE_SPEED : 0.0f, 
        SPIN_ATTACK_ACCELL, 
        PLAYER_STOP_ACCELERATION
    );
    player->tilt = mathfLerp(player->tilt, 0.0f, 0.1f);

    if (playerInputGetDown(input, PlayerInputActionsAttack)) {
        if ((player->flags & (PlayerFlagsInAttackWindow | PlayerFlagsAttackEarly)) == PlayerFlagsInAttackWindow) {
            if (player->attackCollider) {
                // wait until after the current attack is finished
                // before chaining to next attack
                player->flags |= PlayerFlagsAttackSuccess;
            } else {
                playerEnterAttackState(player, factionGetAttack(player->team.teamNumber, player->attackInfo->chainedTo));
            }
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
            damageHandlerInit(&player->damageHandler, gTeamFactions[player->team.teamNumber]->maxHP);
            playerEnterSpawnState(player);
        }
    }

    player->stateTimer -= gTimeDelta;
}

void playerStateWalk(struct Player* player, struct PlayerInput* input) {
    playerRotateTowardsInput(player, input, PLAYER_MAX_ROTATE_SEC);

    struct Vector3 forwardDir;
    quatMultVector(&player->transform.rotation, &gForward, &forwardDir);
    struct Vector3 newVelocity;
    vector3Project(&player->velocity, &forwardDir, &newVelocity);
    struct Vector3 velocityCross;
    vector3Cross(&player->velocity, &newVelocity, &velocityCross);
    
    player->tilt = mathfLerp(player->tilt, velocityCross.y * 0.015f, 0.1f);

    player->velocity = newVelocity;

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
    } else if (input->actionFlags & PlayerInputActionsAttack) {
        playerEnterAttackChargeState(player);
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

            if (player->armature.boneTransforms[0].position.y < 53) {
                player->armature.boneTransforms[0].position.y = 53;
            }
        }
    }

    int hasWalkingSound = soundPlayerIsPlaying(player->walkSoundEffect);

    if (isMoving != hasWalkingSound) {
        if (isMoving) {
            player->walkSoundEffect = soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.walkSounds), 1.0f, 2.0f, SoundPlayerPriorityBackground, SoundPlayerFlagsLooping, &player->transform.position);
            soundPlayerSetVolume(player->walkSoundEffect, mathfLerp(0.125f, 0.35f, player->animationSpeed));
            player->footstepTimer = PLAYER_FOOTSTEP_LEN;
        } else {
            soundPlayerStop(&player->walkSoundEffect);
        }
    }

    if (isMoving) {
        soundPlayerSetPosition(player->walkSoundEffect, &player->transform.position);
    }

    // int hasIdleSound = player->idleSoundEffect != SOUND_ID_NONE;

    // if (isMoving == hasIdleSound) {
    //     if (isMoving) {
    //         soundPlayerStop(&player->idleSoundEffect);
    //     } else {
    //         player->idleSoundEffect = soundPlayerPlay(soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.idleSounds), 1.0f, 0, &player->transform.position);
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

    if (player->touchedBy) {
        player->aiTarget = player->touchedBy;
    } else {
        player->aiTarget = 0;
    }

    player->touchedBy = 0;
}

void playerRender(struct Player* player, struct RenderState* renderState) {
    static struct Coloru8 gPunchColor = {200, 200, 32, 128};
    static struct Coloru8 gTimingWindowColor = {240, 32, 32, 128};
    static struct Coloru8 gKnockbackColor = {240, 120, 32, 128};

    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    struct Quaternion tilt;
    quatAxisAngle(&gForward, player->tilt, &tilt);
    struct Transform finalTranform;
    finalTranform.position = player->transform.position;
    quatMultiply(&player->transform.rotation, &tilt, &finalTranform.rotation);
    finalTranform.scale = player->transform.scale;

    transformToMatrixL(&finalTranform, matrix);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    int usePallete = player->team.teamNumber;
    
    if (player->state == playerStateAttackCharge && player->stateTimer <= 0.0f && mathfMod(gTimePassed, 0.1f) > 0.05f) {
        usePallete = SUPER_SAYAN_PALLETE_INDEX;
    }
    
    if (damageHandlerIsFlashing(&player->damageHandler)) {
        usePallete = DAMAGE_PALLETE_INDEX;
    }

    gDPUseTeamPallete(renderState->dl++, usePallete, 2);
    skRenderObject(&player->armature, renderState);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

    recallCircleRender(&player->recallCircle, renderState, player->team.teamNumber);
    if (player->attackCollider) {
        struct Coloru8 punchColor = gPunchColor;

        if ((player->flags & (PlayerFlagsInAttackWindow | PlayerFlagsAttackEarly)) == PlayerFlagsInAttackWindow) {
            punchColor = gTimingWindowColor;
        } else if (player->attackInfo && player->attackInfo->knockback > 0) {
            punchColor = gKnockbackColor;
        }

        punchTrailRender(&player->punchTrail, renderState, punchColor);
    }
}

void playerApplyDamage(struct Player* player, float amount, struct Vector3* origin, float knockback) {
    if (player->transform.position.y < INVINCIBLE_JUMP_HEIGHT || player->state == playerStateSpawn) {
        teamEntityApplyKnockback(&player->transform.position, &player->velocity, origin, knockback);
        if (damageHandlerApplyDamage(&player->damageHandler, amount, PLAYER_INVINCIBILITY_TIME)) {
            soundPlayerPlay(
                soundListRandom(&gTeamFactions[player->playerIndex]->playerSounds.damageSounds), 
                1.0f, 
                gTeamFactions[player->team.teamNumber]->soundPitchScalar,
                SoundPlayerPriorityPlayer, 
                0, 
                &player->transform.position
            );

            if (knockback > 0.0f && player->state == playerStateWalk) {
                player->state = playerStateFreefall;
                player->animationSpeed = 1.0f;
                player->flags |= PlayerFlagsKnockedBack;
            }
        }
    }
}

int playerIsAlive(struct Player* player) {
    return player->damageHandler.hp > 0 && player->state != playerStateDead;
}

int playerIsSpinAttackReady(struct Player* player) {
    return player->state == playerStateAttackCharge && player->stateTimer <= 0.0f;
}

int playerIsAttacking(struct Player* player) {
    return player->state == playerStateAttack;
}

int playerCanChainAttack(struct Player* player) {
    return player->state == playerStateAttack && (player->flags & PlayerFlagsInAttackWindow) != 0;
}