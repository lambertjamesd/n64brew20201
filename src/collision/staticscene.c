
#include "staticscene.h"
#include "math/mathf.h"
#include "math/vector3.h"
#include "game_defs.h"

#define STATIC_WALL_THICKNESS   (SCENE_SCALE * 1.5f)

void staticSceneConstrainToBoundaries(struct StaticScene* staticScene, struct Vector2* pos, struct Vector2* velocity, float radius) {
    for (unsigned i = 0; i < staticScene->boundaryCount; ++i) {
        struct Vector2 offset;
        struct SceneBoundary* curr = &staticScene->boundary[i];
        vector2Sub(pos, &curr->corner, &offset);
        float intersectionDepth = vector2Dot(&offset, &curr->normal) - radius;

        if (intersectionDepth > 0.0f || intersectionDepth + STATIC_WALL_THICKNESS < -radius * 2.0f) {
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

        if (velocity) {
            struct Vector2 velocityIntoFace;
            vector2Scale(&curr->normal, vector2Dot(&curr->normal, velocity), &velocityIntoFace);
            vector2Sub(velocity, &velocityIntoFace, velocity);
        }
    }
}

int staticSceneInInsideBoundary(struct StaticScene* staticScene, struct Vector2* pos, float radius) {
    unsigned intersectionCount = 0;

    for (unsigned i = 0; i < staticScene->boundaryCount; ++i) {
        struct SceneBoundary* curr = &staticScene->boundary[i];
        struct SceneBoundary* next = &staticScene->boundary[(i + 1) % staticScene->boundaryCount];

        struct Vector2 currPoint;
        struct Vector2 nextPoint;

        vector2Sub(&curr->corner, pos, &currPoint);
        vector2Sub(&next->corner, pos, &nextPoint);

        float yDiff = nextPoint.y - currPoint.y;
        if (fabsf(yDiff) < 0.0001f) {
            continue;
        }

        float lerpValue = -currPoint.y / yDiff;

        if (lerpValue < 0 || lerpValue > 1.0f) {
            continue;
        }

        float xValue = (1.0f - lerpValue) * currPoint.x + lerpValue * nextPoint.x;

        if (xValue > 0.0f) {
            ++intersectionCount;
        }
    }

    if ((intersectionCount & 0x1) == 0) {
        return 0;
    }

    struct Vector2 posCheck = *pos;
    staticSceneConstrainToBoundaries(staticScene, pos, 0, radius);
    return posCheck.x == pos->x && posCheck.y == pos->y;
}