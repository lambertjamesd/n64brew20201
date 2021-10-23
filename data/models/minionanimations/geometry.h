#ifndef _output_H
#define _output_H

#include <ultra64.h>
#include "math/transform.h"
#include "sk64/skelatool_animation_clip.h"

extern Gfx output_model_gfx[];
extern struct Transform output_default_bones[];
#define OUTPUT_DEFAULT_BONES_COUNT 1
extern struct SKAnimationHeader output_animations[];

#endif