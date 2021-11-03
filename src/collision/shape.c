
#include "shape.h"
#include "circle.h"
#include "polygon.h"

CollisionFunction collisionFunctionTable[CollisionShapeTypeCount * CollisionShapeTypeCount] = {
    collisionCircleCircle, collisionCirclePolygon,
    collisionCirclePolygon, 0,
};

void collisionShapeBoundingBox(struct CollisionShape* shape, struct Vector2* at, struct Box2D* output) {
    switch (shape->type)
    {
        case CollisionShapeTypePolygon:
            collisionPolygonBoundingBox((struct CollisionPolygon*)shape, at, output);
            break;
        case CollisionShapeTypeCircle:
            collisionCircleBoundingBox((struct CollisionCircle*)shape, at, output);
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