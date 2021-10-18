
#include "shape.h"
#include "circle.h"

CollisionFunction collisionFunctionTable[CollisionShapeTypeCount * CollisionShapeTypeCount] = {
    collisionCircleCircle, 0,
    0, 0,
};

void collisionShapeBoundingBox(struct CollisionShape* shape, struct Vector2* at, struct Box2D* outuput) {
    switch (shape->type)
    {
        case CollisionShapeTypeCircle:
            collisionCircleBoundingBox((struct CollisionCircle*)shape, at, outuput);
            break;
    }

    outuput->max = *at;
    outuput->max = *at;
}

int collisionCollidePair(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap) {
    int didFlip = 0;
    if (a->type < b->type) {
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