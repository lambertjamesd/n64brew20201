
#include "shape.h"
#include "circle.h"
#include "polygon.h"
#include "util/rom.h"

CollisionFunction collisionFunctionTable[CollisionShapeTypeCount * CollisionShapeTypeCount] = {
    collisionCircleCircle, collisionCirclePolygon,
    collisionCirclePolygon, 0,
};

void collisionShapeBoundingBox(struct CollisionShape* shape, struct Vector2* at, struct Vector2* rotation, float scale, struct Box2D* output) {
    switch (shape->type)
    {
        case CollisionShapeTypePolygon:
            collisionPolygonBoundingBox((struct CollisionPolygon*)shape, at, rotation, scale, output);
            break;
        case CollisionShapeTypeCircle:
            collisionCircleBoundingBox((struct CollisionCircle*)shape, at, rotation, scale, output);
            break;
        default:
            output->max = *at;
            output->max = *at;
            break;
    }
}

int collisionCollidePair(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap) {
    int didFlip = 0;
    if (a->type > b->type) {
        struct CollisionShape* tmp = a;
        a = b;
        b = tmp;
        vector2Negate(aToB, aToB);
        didFlip = 1;
    }

    CollisionFunction fn = collisionFunctionTable[a->type * CollisionShapeTypeCount + b->type];

    if (fn) {
        int result = fn(a, b, aToB, shapeOverlap);

        if (result && didFlip && shapeOverlap) {
            shapeOverlap->depth = -shapeOverlap->depth;
        }

        return result;
    }
    
    return 0;
}

struct CollisionShape* collisionShapeUnpack(struct CollisionShape* shape, void* segmentRamStart) {
    if (!shape) {
        return 0;
    }

    struct CollisionShape* result = CALC_RAM_POINTER(shape, segmentRamStart);
    if (result->type == CollisionShapeTypePolygon) {
        struct CollisionPolygon* asPolygon = (struct CollisionPolygon*)result;
        asPolygon->edges = CALC_RAM_POINTER(asPolygon->edges, segmentRamStart);
    }
    return result;
}