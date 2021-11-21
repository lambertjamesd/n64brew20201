#include "polygon.h"
#include "circle.h"
#include "math/mathf.h"

#define EDGE_LERP_TOLERANCE 0.1f

void collisionPolygonBoundingBox(struct CollisionPolygon* shape, struct Vector2* at, struct Vector2* rotation, float scale, struct Box2D* outuput) {
    vector2ComplexMul(&shape->edges[0].corner, rotation, &outuput->min);
    outuput->max = outuput->min;

    for (unsigned i = 1; i < shape->edgeCount; ++i) {
        struct Vector2 edgePos;
        vector2ComplexMul(&shape->edges[i].corner, rotation, &edgePos);
        vector2Min(&outuput->min, &edgePos, &outuput->min);
        vector2Max(&outuput->max, &edgePos, &outuput->max);
    }

    vector2Scale(&outuput->min, scale, &outuput->min);
    vector2Scale(&outuput->max, scale, &outuput->max);

    vector2Add(&outuput->min, at, &outuput->min);
    vector2Add(&outuput->max, at, &outuput->max);
}

int collisionCirclePolygonPoint(struct CollisionCircle* aAsCircle, struct CollisionPolygonEdge* currEdge, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap) {
    struct Vector2 offset;
    vector2Add(&currEdge->corner, aToB, &offset);
    float pointDistance = vector2DistSqr(&currEdge->corner, &offset);
    
    if (pointDistance < aAsCircle->radius * aAsCircle->radius) {
        if (shapeOverlap) {
            float distance = sqrtf(pointDistance);

            if (distance < 0.00001f) {
                vector2Negate(&currEdge->normal, &shapeOverlap->normal);
                shapeOverlap->depth  = aAsCircle->radius;
            } else {
                vector2Scale(aToB, 1.0f / distance, &shapeOverlap->normal);
                shapeOverlap->depth = aAsCircle->radius - distance;
            }
        }

        return 1;
    }

    return 0;
}

int collisionCirclePolygon(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap) {
    struct CollisionCircle* aAsCircle = (struct CollisionCircle*)a;
    struct CollisionPolygon* bAsPolygon = (struct CollisionPolygon*)b;

    float minOverlap = 0.0f;
    unsigned edgeIndex = bAsPolygon->edgeCount;
    int wasFirstLerpBefore = 0;
    int wasPrevLerpAfter = 0;

    struct CollisionPolygonEdge* currEdge; 

    for (currEdge = bAsPolygon->edges; currEdge < bAsPolygon->edges + bAsPolygon->edgeCount; ++currEdge) {
        // check edge
        struct Vector2 offset;
        vector2Add(&currEdge->corner, aToB, &offset);
        float planeOverlap = aAsCircle->radius + vector2Dot(&offset, &currEdge->normal);
        
        if (planeOverlap < 0.0f) {
            return 0;
        }

        float edgeLerp = vector2Cross(&offset, &currEdge->normal);

        unsigned isLerpBefore = edgeLerp < EDGE_LERP_TOLERANCE;
        unsigned isLerpAfter = edgeLerp > currEdge->edgeLen - EDGE_LERP_TOLERANCE;

        if (currEdge == bAsPolygon->edges) {
            wasFirstLerpBefore = isLerpBefore;
        } else {
            if (isLerpBefore && wasPrevLerpAfter) {
                return collisionCirclePolygonPoint(aAsCircle, currEdge, aToB, shapeOverlap);
            }

            wasPrevLerpAfter = isLerpAfter;
        }

        if (!isLerpBefore && !isLerpAfter && (edgeIndex == bAsPolygon->edgeCount || planeOverlap < minOverlap)) {
            minOverlap = planeOverlap;
            edgeIndex = currEdge - bAsPolygon->edges;
        }
    }

    if (wasFirstLerpBefore && wasPrevLerpAfter) {
        return collisionCirclePolygonPoint(aAsCircle, bAsPolygon->edges, aToB, shapeOverlap);
    }

    if (edgeIndex < bAsPolygon->edgeCount) {
        if (shapeOverlap) {
            shapeOverlap->depth = minOverlap;
            vector2Negate(&bAsPolygon->edges[edgeIndex].normal, &shapeOverlap->normal);
        }
        return 1;
    }

    return 0;
}