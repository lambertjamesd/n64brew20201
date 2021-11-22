#include "faction.h"
#include "util/util.h"
#include "../data/models/characters.h"

#include "../data/models/catlow/geometry_animdef.inc.h"
#include "../data/models/doglow/geometry_animdef.inc.h"

struct PlayerAttackInfo gDogAttacks[] = {
    [PlayerAttackPunch] = {
        DOGLOW_ARM1_BONE, 
        1,
        1.5f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
        &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_001_INDEX],
    },
    [PlayerAttackPunchChain] = {
        DOGLOW_ARM2_BONE, 
        0,
        2.0f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
        &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_002_INDEX],
    },
    [PlayerAttackJumpAttack] = {
        DOGLOW_TAIL_BONE, 
        0,
        0.5f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 1.5f * SCENE_SCALE},
        &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_JUMP_ATTACK_INDEX],
    },
};

struct SKAnimationHeader* gDogAnimations[] = {
    [PlayerAnimationIdle] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_IDLE_INDEX],
    [PlayerAnimationWalk] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_WALK_INDEX],
    [PlayerAnimationDie] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_DIE_INDEX],
    [PlayerAnimationJump] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_JUMP_INDEX],
    [PlayerAnimationFall] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_FALL_INDEX],
    [PlayerAnimationJumpAttack] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_JUMP_ATTACK_INDEX],
    [PlayerAnimationJumpAttackLanding] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_JUMP_ATTACK_LANDING_INDEX],
    [PlayerAnimationSelectIdle] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_WALK_INDEX],
    [PlayerAnimationSelected] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_002_INDEX],
    [PlayerAnimationVictory] = &doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_VICTORY_INDEX],
};

struct SKAnimationEvent gDogAttack001Events[] = {
    {9, PLAYER_ATTACK_START_ID},
    {14, PLAYER_ATTACK_WINDOW_ID},
    {15, PLAYER_ATTACK_END_ID},
};

struct SKAnimationEvent gDogAttack002Events[] = {
    {4, PLAYER_ATTACK_START_ID},
    {8, PLAYER_ATTACK_END_ID},
};

unsigned short gDogDamageSounds[] = {SOUNDS_DOG_INJURED_GRUNT_1, SOUNDS_DOG_INJURED_GRUNT_2};
unsigned short gDogAttackSounds[] = {SOUNDS_DOG_ATTACK1, SOUNDS_DOG_ATTACK2, SOUNDS_DOG_ATTACK3};
unsigned short gDogDeathSounds[] = {SOUNDS_DOG_INJURED_GRUNT_2};
unsigned short gDogJumpSounds[] = {
    SOUNDS_DOG_JUMP_GRUNT_1, 
    SOUNDS_DOG_JUMP_GRUNT_2, 
    SOUNDS_DOG_JUMP_GRUNT_3, 
    SOUNDS_DOG_JUMP_GRUNT_4, 
    SOUNDS_DOG_JUMP_GRUNT_5, 
    SOUNDS_DOG_JUMP_GRUNT_6, 
    SOUNDS_DOG_JUMP_GRUNT_7
};
unsigned short gDogIdleSounds[] = {SOUNDS_DOG_PANT_IDLE_1, SOUNDS_DOG_PANT_IDLE_2};

struct Faction gDogFaction = {
    .name = "Dogs",
    .playerAttacks = gDogAttacks,
    .playerAnimations = gDogAnimations,
    .playerDefaultPose = doglow_default_bones,
    .playerBoneParent = doglow_bone_parent,
    .playerBoneCount = DOGLOW_DEFAULT_BONES_COUNT,
    .playerMesh = doglow_DogLow_mesh,
    .playerRotationCorrection = {1.0f, 0.0f},
    .playerSounds = {
        .damageSounds = {.options = gDogDamageSounds, .count = sizeof(gDogDamageSounds)/sizeof(gDogDamageSounds[0])},
        .attackSounds = {.options = gDogAttackSounds, .count = sizeof(gDogAttackSounds)/sizeof(gDogAttackSounds[0])},
        .deathSounds = {.options = gDogDeathSounds, .count = sizeof(gDogDeathSounds)/sizeof(gDogDeathSounds[0])},
        .jumpSounds = {.options = gDogJumpSounds, .count = sizeof(gDogJumpSounds)/sizeof(gDogJumpSounds[0])},
        .idleSounds = {.options = gDogIdleSounds, .count = sizeof(gDogIdleSounds)/sizeof(gDogIdleSounds[0])},
        .walkSound = SOUNDS_DOG_WALKING_LOOP_1,
    },
    .moveSpeed = PLAYER_BASE_MOVE_SPEED,
    .accel = 20.0f,

    .minionMesh = Minion_DogMinion_mesh,
};

struct PlayerAttackInfo gCatAttacks[] = {
    [PlayerAttackPunch] = {
        CATLOW_ARM1_BONE, 
        1,
        1.5f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
        &catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_004_INDEX],
    },
    [PlayerAttackPunchChain] = {
        CATLOW_BOOT2_BONE, 
        0,
        2.0f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 0.5f * SCENE_SCALE},
        &catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_003_INDEX],
    },
    [PlayerAttackJumpAttack] = {
        CATLOW_BOOT1_BONE, 
        0,
        0.5f,
        {0.0f, 0.65f * SCENE_SCALE, 0.0f}, 
        {{CollisionShapeTypeCircle}, 1.5f * SCENE_SCALE},
        &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_INDEX],
    },
};

struct SKAnimationHeader* gCatAnimations[] = {
    [PlayerAnimationIdle] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_IDLE_INDEX],
    [PlayerAnimationWalk] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_WALK_INDEX],
    [PlayerAnimationDie] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_DIE_INDEX],
    [PlayerAnimationJump] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_INDEX],
    [PlayerAnimationFall] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_FALL_INDEX],
    [PlayerAnimationJumpAttack] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_INDEX],
    [PlayerAnimationJumpAttackLanding] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_LANDING_INDEX],
    [PlayerAnimationSelectIdle] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_WALK_INDEX],
    [PlayerAnimationSelected] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_003_INDEX],
    [PlayerAnimationVictory] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_VICTORY_INDEX],
};

struct SKAnimationEvent gCatAttack001Events[] = {
    {7, PLAYER_ATTACK_START_ID},
    {12, PLAYER_ATTACK_END_ID},
    {14, PLAYER_ATTACK_WINDOW_ID},
};

struct SKAnimationEvent gCatAttack002Events[] = {
    {9, PLAYER_ATTACK_START_ID},
    {12, PLAYER_ATTACK_END_ID},
};

unsigned short gCatDamageSounds[] = {SOUNDS_DOG_INJURED_GRUNT_1};
unsigned short gCatAttackSounds[] = {SOUNDS_DOG_ATTACK1};
unsigned short gCatDeathSounds[] = {SOUNDS_CATMEOW1};
unsigned short gCatJumpSounds[] = {SOUNDS_DOG_JUMP_GRUNT_1};
unsigned short gCatIdlesSounds[] = {SOUNDS_CATMEOW1};

struct Faction gCatFaction = {
    .name = "Cats",
    .playerAttacks = gCatAttacks,
    .playerAnimations = gCatAnimations,
    .playerDefaultPose = catlow_default_bones,
    .playerBoneParent = catlow_bone_parent,
    .playerBoneCount = CATLOW_DEFAULT_BONES_COUNT,
    .playerMesh = catlow_CatLow_mesh,
    .playerRotationCorrection = {1.0f, 0.0f},
    .playerSounds = {
        .damageSounds = {.options = gCatDamageSounds, .count = sizeof(gCatDamageSounds)/sizeof(gCatDamageSounds[0])},
        .attackSounds = {.options = gCatAttackSounds, .count = sizeof(gCatAttackSounds)/sizeof(gCatAttackSounds[0])},
        .deathSounds = {.options = gCatDeathSounds, .count = sizeof(gCatDeathSounds)/sizeof(gCatDeathSounds[0])},
        .jumpSounds = {.options = gCatJumpSounds, .count = sizeof(gCatJumpSounds)/sizeof(gCatJumpSounds[0])},
        .idleSounds = {.options = gCatIdlesSounds, .count = sizeof(gCatIdlesSounds)/sizeof(gCatIdlesSounds[0])},
        .walkSound = SOUNDS_DOG_WALKING_LOOP_1,
    },
    .moveSpeed = PLAYER_BASE_MOVE_SPEED,
    .accel = 20.0f,

    .minionMesh = Minion_CatMinion_mesh,
};

// can't have a null pointer exception if gTeamFactions never point to null
struct Faction* gTeamFactions[MAX_PLAYERS] = {
    &gDogFaction,
    &gDogFaction,
    &gDogFaction,
    &gDogFaction,
};

struct Faction* gFactions[FACTION_COUNT] = {
    &gCatFaction,
    &gDogFaction,
};

void factionGlobalInit() {
    doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_001_INDEX].numEvents = COUNT_OF(gDogAttack001Events);
    doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_001_INDEX].animationEvents = gDogAttack001Events;

    doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_002_INDEX].numEvents = COUNT_OF(gDogAttack002Events);
    doglow_animations[DOGLOW_DOGLOW_ARMATURE_001_PUNCH_002_INDEX].animationEvents = gDogAttack002Events;

    catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_004_INDEX].numEvents = COUNT_OF(gCatAttack001Events);
    catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_004_INDEX].animationEvents = gCatAttack001Events;

    catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_003_INDEX].numEvents = COUNT_OF(gCatAttack002Events);
    catlow_animations[CATLOW_CATLOW_ARMATURE_001_PUNCH_003_INDEX].animationEvents = gCatAttack002Events;
}

struct SKAnimationHeader* factionGetAnimation(unsigned team, enum PlayerAnimation anim) {
    return gTeamFactions[team]->playerAnimations[anim];
}

struct PlayerAttackInfo* factionGetAttack(unsigned team, enum PlayerAttack attack) {
    return &gTeamFactions[team]->playerAttacks[attack];
}