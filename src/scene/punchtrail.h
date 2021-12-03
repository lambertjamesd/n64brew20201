#ifndef _SCENE_PUNCH_TRAIL_H
#define _SCENE_PUNCH_TRAIL_H

#include "../data/models/punchtrail/geometry.h"

#include "math/transform.h"
#include "graphics/render_state.h"

struct PunchTrail {
    struct Transform segmentTransforms[PUNCHTRAIL_DEFAULT_BONES_COUNT];
};

void punchTrailInit(struct PunchTrail* punchTrail, struct Vector3* start, float scale);
void punchTrailUpdate(struct PunchTrail* punchTrail, struct Vector3* next);
void punchTrailRender(struct PunchTrail* punchTrail, struct RenderState* renderState, struct Coloru8 color);
struct Vector3* punchTrailHeadPosition(struct PunchTrail* punchTrail);

#endif