
#include "circle.h"

#include "math/mathf.h"

void collisionCircleBoundingBox(struct CollisionCircle* shape, struct Vector2* at, struct Vector2* rotation, float scale, struct Box2D* outuput) {
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
        if (shapeOverlap) {
            float distance = sqrtf(distSqr);

            if (distance <= 0.0001f) {
                shapeOverlap->normal = gRight2;
                shapeOverlap->depth = radiusSum;
            } else {
                vector2Scale(aToB, 1.0f / distance, &shapeOverlap->normal);
                shapeOverlap->depth = radiusSum - distance;
            }
        }

        return 1;
    }

    return 0;
}