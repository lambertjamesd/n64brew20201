
#include "circle.h"

void collisionCircleBoundingBox(struct CollisionCircle* shape, struct Vector2* at, struct Box2D* outuput) {
    outuput->min.x = at->x - shape->radius;
    outuput->min.y = at->y - shape->radius;
    outuput->max.x = at->x + shape->radius;
    outuput->max.y = at->y + shape->radius;
}

int collisionCircleCircle(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap) {
    struct CollisionCircle* aAsCircle = (struct CollisionCircle*)a;
    struct CollisionCircle* bAsCircle = (struct CollisionCircle*)b;

    float radiusSum = aAsCircle->radius + bAsCircle->radius;
    float distSqr = vector2MagSqr(aToB);

    if (distSqr <= radiusSum * radiusSum) {
        return 1;
    }

    return 0;
}