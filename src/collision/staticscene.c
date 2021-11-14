
#include "staticscene.h"

void staticSceneConstrainToBoundaries(struct StaticScene* staticScene, struct Vector2* pos, struct Vector2* velocity, float radius) {
    for (unsigned i = 0; i < staticScene->boundaryCount; ++i) {
        struct Vector2 offset;
        vector2Sub(pos, &staticScene->boundary[i].at, &offset);
        float intersectionDepth = vector2Dot(&offset, &staticScene->boundary[i].normal) - radius;

        if (intersectionDepth < 0.0f) {
            struct Vector2 offsetCorrection;
            vector2Scale(&staticScene->boundary[i].normal, -intersectionDepth, &offsetCorrection);
            vector2Add(pos, &offsetCorrection, pos);

            struct Vector2 velocityIntoFace;
            vector2Scale(&staticScene->boundary[i].normal, vector2Dot(&staticScene->boundary[i].normal, velocity), &velocityIntoFace);
            vector2Sub(velocity, &velocityIntoFace, velocity);
        }
    }
}

int staticSceneInInsideBoundary(struct StaticScene* staticScene, struct Vector2* pos, float radius) {
    struct Vector2 velocity = gZeroVec2;
    struct Vector2 posCheck = *pos;
    staticSceneConstrainToBoundaries(staticScene, pos, &velocity, radius);
    return posCheck.x == pos->x && posCheck.y == pos->y;
}