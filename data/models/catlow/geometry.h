#ifndef _catlow_H
#define _catlow_H

#include <ultra64.h>
#include "math/transform.h"
#include "sk64/skelatool_clip.h"

extern Gfx catlow_model_gfx[];
#define CATLOW_ROOT_BONE 0
#define CATLOW_SHOULDER1_BONE 1
#define CATLOW_ARM1_BONE 2
#define CATLOW_THIGH1_BONE 3
#define CATLOW_BOOT1_BONE 4
#define CATLOW_HEAD_BONE 5
#define CATLOW_TAIL_BONE 6
#define CATLOW_SHOULDER2_BONE 7
#define CATLOW_ARM2_BONE 8
#define CATLOW_THIGH2_BONE 9
#define CATLOW_BOOT2_BONE 10
#define CATLOW_DEFAULT_BONES_COUNT 11
extern struct Transform catlow_default_bones[];
extern unsigned short catlow_bone_parent[];
#define CATLOW_CATLOW_ARMATURE_001_CROUCH_INDEX 0
#define CATLOW_CATLOW_ARMATURE_001_CROUCH_TO_STAND_INDEX 1
#define CATLOW_CATLOW_ARMATURE_001_DASH_INDEX 2
#define CATLOW_CATLOW_ARMATURE_001_DASH_ATTACK_INDEX 3
#define CATLOW_CATLOW_ARMATURE_001_DIE_INDEX 4
#define CATLOW_CATLOW_ARMATURE_001_FALL_INDEX 5
#define CATLOW_CATLOW_ARMATURE_001_IDLE_INDEX 6
#define CATLOW_CATLOW_ARMATURE_001_JUMP_INDEX 7
#define CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_INDEX 8
#define CATLOW_CATLOW_ARMATURE_001_JUMP_ATTACK_LANDING_INDEX 9
#define CATLOW_CATLOW_ARMATURE_001_KICK_INDEX 10
#define CATLOW_CATLOW_ARMATURE_001_LOW_PUNCH_003_INDEX 11
#define CATLOW_CATLOW_ARMATURE_001_LOW_PUNCH_004_INDEX 12
#define CATLOW_CATLOW_ARMATURE_001_PUNCH_006_INDEX 13
#define CATLOW_CATLOW_ARMATURE_001_PUNCH_007_INDEX 14
#define CATLOW_CATLOW_ARMATURE_001_PUNCH_008_INDEX 15
#define CATLOW_CATLOW_ARMATURE_001_SPAWN_INDEX 16
#define CATLOW_CATLOW_ARMATURE_001_SPIN_ATTACK_INDEX 17
#define CATLOW_CATLOW_ARMATURE_001_T_POSE_001_INDEX 18
#define CATLOW_CATLOW_ARMATURE_001_VICTORY_INDEX 19
#define CATLOW_CATLOW_ARMATURE_001_WALK_INDEX 20
extern struct SKAnimationHeader catlow_animations[];

#endif