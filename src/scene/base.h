#ifndef _SCENE_BASE_H
#define _SCENE_BASE_H

#include "math/vector2.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"

struct LevelBase {
    struct Vector2 position;
    struct DynamicSceneEntry* collider;
};

void levelBaseInit(struct LevelBase* base, struct Vector2* position);
void levelBaseUpdate(struct LevelBase* base);
void levelBaseRender(struct LevelBase* base, struct RenderState* renderState);

#endif