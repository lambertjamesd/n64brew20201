#ifndef _COLLISION_CIRCLE_H
#define _COLLISION_CIRCLE_H

#include "math/vector2.h"
#include "shape.h"

struct CollisionCircle {
    struct CollisionShape shapeCommon;
    float radius;
};

void collisionCircleBoundingBox(struct CollisionCircle* shape, struct Vector2* at, struct Vector2* rotation, float scale, struct Box2D* outuput);

int collisionCircleCircle(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap);

#endif 