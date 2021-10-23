#ifndef _doglow_H
#define _doglow_H

#include <ultra64.h>
#include "math/transform.h"
#include "sk64/skelatool_clip.h"

extern Gfx doglow_model_gfx[];
extern struct Transform doglow_default_bones[];
#define DOGLOW_DEFAULT_BONES_COUNT 14
extern struct SKAnimationHeader doglow_animations[];

#endif