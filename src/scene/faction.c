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
        DOGLOW_BOOT1_BONE, 
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
};

struct SKAnimationEvent gDogAttack001Events[] = {
    {9, PLAYER_ATTACK_START_ID},
    {11, PLAYER_ATTACK_WINDOW_ID},
    {12, PLAYER_ATTACK_END_ID},
};

struct SKAnimationEvent gDogAttack002Events[] = {
    {4, PLAYER_ATTACK_START_ID},
    {8, PLAYER_ATTACK_END_ID},
};

struct Faction gDogFaction = {
    .playerAttacks = gDogAttacks,
    .playerAnimations = gDogAnimations,
    .playerDefaultPose = doglow_default_bones,
    .playerBoneParent = doglow_bone_parent,
    .playerBoneCount = DOGLOW_DEFAULT_BONES_COUNT,
    .playerMesh = doglow_DogLow_mesh,
    .playerRotationCorrection = {1.0f, 0.0f},
    .moveSpeed = 16.0f,
    .accel = 40.0f,

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
    [PlayerAnimationFall] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_PEAK_INDEX],
    [PlayerAnimationJumpAttack] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_INDEX],
    [PlayerAnimationJumpAttackLanding] = &catlow_animations[CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_LANDING_INDEX],
};

struct SKAnimationEvent gCatAttack001Events[] = {
    {9, PLAYER_ATTACK_START_ID},
    {13, PLAYER_ATTACK_WINDOW_ID},
    {14, PLAYER_ATTACK_END_ID},
};

struct SKAnimationEvent gCatAttack002Events[] = {
    {9, PLAYER_ATTACK_START_ID},
    {12, PLAYER_ATTACK_END_ID},
};

struct Faction gCatFaction = {
    .playerAttacks = gCatAttacks,
    .playerAnimations = gCatAnimations,
    .playerDefaultPose = catlow_default_bones,
    .playerBoneParent = catlow_bone_parent,
    .playerBoneCount = CATLOW_DEFAULT_BONES_COUNT,
    .playerMesh = catlow_CatLow_mesh,
    .playerRotationCorrection = {1.0f, 0.0f},
    .moveSpeed = 16.0f,
    .accel = 40.0f,

    .minionMesh = Minion_CatMinion_mesh,
};

// can't have a null pointer exception if gTeamFactions never point to null
struct Faction* gTeamFactions[MAX_PLAYERS] = {
    &gCatFaction,
    &gDogFaction,
    &gDogFaction,
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