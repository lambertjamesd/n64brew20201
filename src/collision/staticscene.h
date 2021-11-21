#ifndef _COLLISION_STATIC_SCENE_H
#define _COLLISION_STATIC_SCENE_H

#include "math/vector2.h"

struct SceneBoundary {
    struct Vector2 corner;
    struct Vector2 normal;
    float length;
};

struct StaticScene {
    struct SceneBoundary* boundary;
    unsigned boundaryCount;
};

void staticSceneConstrainToBoundaries(struct StaticScene* staticScene, struct Vector2* pos, struct Vector2* velocity, float radius);
int staticSceneInInsideBoundary(struct StaticScene* staticScene, struct Vector2* pos, float radius);

#endif