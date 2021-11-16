#ifndef _FACTION_H
#define _FACTION_H

#include <ultra64.h>

#include "audio/clips.h"
#include "collision/circle.h"
#include "math/vector3.h"
#include "game_defs.h"
#include "sk64/skelatool_clip.h"
#include "math/vector2.h"
#include "math/mathf.h"
#include "audio/soundplayer.h"

struct PlayerAttackInfo {
    unsigned char boneIndex;
    unsigned char chainedTo;
    float damage;
    struct Vector3 localPosition;
    struct CollisionCircle collisionCircle;
    struct SKAnimationHeader* animation;
};

enum PlayerAttack {
    PlayerAttackPunch,
    PlayerAttackPunchChain,
    PlayerAttackJumpAttack,
};

#define PLAYER_ATTACK_START_ID                     0x0
#define PLAYER_ATTACK_END_ID                       0x1
#define PLAYER_ATTACK_WINDOW_ID                    0x2

#define PLAYER_BASE_MOVE_SPEED                     16.0f

#define FACTION_COUNT   2

enum PlayerAnimation {
    PlayerAnimationIdle,
    PlayerAnimationWalk,
    PlayerAnimationDie,
    PlayerAnimationJump,
    PlayerAnimationFall,
    PlayerAnimationJumpAttack,
    PlayerAnimationJumpAttackLanding,
    PlayerAnimationSelectIdle,
    PlayerAnimationSelected,
};

struct PlayerFactionSounds {
    struct SoundList damageSounds;
    struct SoundList attackSounds;
    struct SoundList deathSounds;
    struct SoundList jumpSounds;
    struct SoundList idleSounds;
    unsigned short walkSound;
};

struct Faction {
    char* name;
    struct PlayerAttackInfo* playerAttacks;
    struct SKAnimationHeader** playerAnimations;
    struct Transform* playerDefaultPose;
    unsigned short* playerBoneParent;
    unsigned short playerBoneCount;
    Gfx* playerMesh;
    struct Vector2 playerRotationCorrection;
    struct PlayerFactionSounds playerSounds;
    float moveSpeed;
    float accel;

    Gfx* minionMesh;
};

extern struct Faction* gTeamFactions[MAX_PLAYERS];
extern struct Faction* gFactions[FACTION_COUNT];

void factionGlobalInit();

struct SKAnimationHeader* factionGetAnimation(unsigned team, enum PlayerAnimation anim);
struct PlayerAttackInfo* factionGetAttack(unsigned team, enum PlayerAttack attack);

#define ANY_FACTION_BONE_COUNT  MAX(DOGLOW_DEFAULT_BONES_COUNT, CATLOW_DEFAULT_BONES_COUNT)

#endif