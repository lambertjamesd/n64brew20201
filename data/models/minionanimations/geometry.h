#ifndef _minion_animations_H
#define _minion_animations_H

#include <ultra64.h>
#include "math/transform.h"
#include "sk64/skelatool_clip.h"

#define MINION_ANIMATIONS_BONE_BONE 0
#define MINION_ANIMATIONS_DEFAULT_BONES_COUNT 1
extern struct Transform minion_animations_default_bones[];
extern unsigned short minion_animations_bone_parent[];
#define MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_ATTACK_INDEX 0
#define MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_DIE_INDEX 1
#define MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_IDLE_INDEX 2
#define MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_MOVEFORWARD_INDEX 3
#define MINION_ANIMATIONS_MINION_ANIMATIONS_ARMATURE_SPAWN_INDEX 4
extern struct SKAnimationHeader minion_animations_animations[];

#endif