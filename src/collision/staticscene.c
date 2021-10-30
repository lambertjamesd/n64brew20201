
#include "staticscene.h"

void staticSceneConstrainToBoundaries(struct StaticScene* staticScene, struct Vector2* pos, float radius) {
    for (unsigned i = 0; i < staticScene->boundaryCount; ++i) {
        struct Vector2 offset;
        vector2Sub(pos, &staticScene->boundary[i].at, &offset);
        float intersectionDepth = vector2Dot(&offset, &staticScene->boundary[i].normal) - radius;

        if (intersectionDepth < 0.0f) {
            vector2Scale(&staticScene->boundary[i].normal, -intersectionDepth, &offset);
            vector2Add(pos, &offset, pos);
        }
    }
}