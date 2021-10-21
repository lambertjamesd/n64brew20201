#ifndef _SCENE_RECALL_CIRCLE_H
#define _SCENE_RECALL_CIRCLE_H

#include "math/vector2.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"

struct RecallCircle {
    struct DynamicSceneEntry* collider;
};

void recallCircleInit(struct RecallCircle* circle);
void recallCircleActivate(struct RecallCircle* circle, struct Vector2* at, unsigned team);
void recallCircleDisable(struct RecallCircle* circle);
void recallCircleRender(struct RecallCircle* circle, struct RenderState* renderState, unsigned team);

#endif