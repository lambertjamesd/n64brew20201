#include "polygon.h"
#include "circle.h"
#include "math/mathf.h"

void collisionPolygonBoundingBox(struct CollisionPolygon* shape, struct Vector2* at, struct Box2D* outuput) {
    vector2Add(&shape->boundingBox.min, at, &outuput->min);
    vector2Add(&shape->boundingBox.max, at, &outuput->max);
}

int collisionCirclePolygon(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap) {
    struct CollisionCircle* aAsCircle = (struct CollisionCircle*)a;
    struct CollisionPolygon* bAsPolygon = (struct CollisionPolygon*)b;

    float minOverlap = 10000000.0f;
    unsigned edgeIndex = bAsPolygon->edgeCount;

    struct CollisionPolygonEdge* currEdge; 

    for (currEdge = bAsPolygon->edges; currEdge < bAsPolygon->edges + bAsPolygon->edgeCount; ++currEdge) {
        struct Vector2 offset;
        vector2Add(&currEdge->corner, aToB, &offset);
        float planeOverlap = aAsCircle->radius + vector2Dot(&offset, &currEdge->normal);
        
        if (planeOverlap < 0.0f) {
            return 0;
        }

        float edgeLerp = vector2Cross(&offset, &currEdge->normal);

        if (edgeLerp >= 0.0f && edgeLerp <= currEdge->edgeLen && planeOverlap < minOverlap) {
            minOverlap = planeOverlap;
            edgeIndex = currEdge - bAsPolygon->edges;
        }
    }

    if (edgeIndex < bAsPolygon->edgeCount) {
        shapeOverlap->depth = minOverlap;
        vector2Negate(&bAsPolygon->edges[edgeIndex].normal, &shapeOverlap->normal);
        return 1;
    }

    float rSqrd = aAsCircle->radius * aAsCircle->radius;

    for (currEdge = bAsPolygon->edges; currEdge < bAsPolygon->edges + bAsPolygon->edgeCount; ++currEdge) {
        struct Vector2 offset;
        vector2Add(&currEdge->corner, aToB, &offset);
        float pointDistance = vector2DistSqr(&currEdge->corner, &offset);
        
        if (pointDistance < rSqrd) {
            float distance = sqrtf(pointDistance);

            if (distance < 0.00001f) {
                vector2Negate(&currEdge->normal, &shapeOverlap->normal);
                shapeOverlap->depth  = aAsCircle->radius;
            } else {
                vector2Scale(aToB, 1.0f / distance, &shapeOverlap->normal);
                shapeOverlap->depth = aAsCircle->radius - distance;
            }
        }
    }

    return 0;
}