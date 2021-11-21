
#include "staticscene.h"

void staticSceneConstrainToBoundaries(struct StaticScene* staticScene, struct Vector2* pos, struct Vector2* velocity, float radius) {
    for (unsigned i = 0; i < staticScene->boundaryCount; ++i) {
        struct Vector2 offset;
        struct SceneBoundary* curr = &staticScene->boundary[i];
        vector2Sub(pos, &curr->corner, &offset);
        float intersectionDepth = vector2Dot(&offset, &curr->normal) - radius;

        if (intersectionDepth > 0.0f || intersectionDepth < -radius * 2.0f) {
            continue;
        }

        float edgeLerp = vector2Cross(&offset, &curr->normal);

        if (edgeLerp < 0.0f) {
            if (vector2MagSqr(&offset) < radius * radius) {
                vector2Normalize(&offset, &offset);
                vector2Scale(&offset, radius, &offset);
                vector2Add(&curr->corner, &offset, pos);
            }

            continue;
        } else if (edgeLerp > curr->length) {
            continue;
        }

        struct Vector2 offsetCorrection;
        vector2Scale(&curr->normal, -intersectionDepth, &offsetCorrection);
        vector2Add(pos, &offsetCorrection, pos);

        struct Vector2 velocityIntoFace;
        vector2Scale(&curr->normal, vector2Dot(&curr->normal, velocity), &velocityIntoFace);
        vector2Sub(velocity, &velocityIntoFace, velocity);
    }
}

int staticSceneInInsideBoundary(struct StaticScene* staticScene, struct Vector2* pos, float radius) {
    // TODO make work with non convex outline
    struct Vector2 velocity = gZeroVec2;
    struct Vector2 posCheck = *pos;
    staticSceneConstrainToBoundaries(staticScene, pos, &velocity, radius);
    return posCheck.x == pos->x && posCheck.y == pos->y;
}