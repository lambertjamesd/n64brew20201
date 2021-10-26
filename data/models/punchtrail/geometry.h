#ifndef _punchtrail_H
#define _punchtrail_H

#include <ultra64.h>
#include "math/transform.h"
#include "sk64/skelatool_clip.h"

extern Gfx punchtrail_model_gfx[];
#define PUNCHTRAIL_BONE_BONE 0
#define PUNCHTRAIL_BONE_001_BONE 1
#define PUNCHTRAIL_BONE_002_BONE 2
#define PUNCHTRAIL_BONE_003_BONE 3
#define PUNCHTRAIL_BONE_004_BONE 4
#define PUNCHTRAIL_DEFAULT_BONES_COUNT 5
extern struct Transform punchtrail_default_bones[];
extern unsigned short punchtrail_bone_parent[];
extern struct SKAnimationHeader punchtrail_animations[];

#endif