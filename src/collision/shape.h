#ifndef _COLLISION_SHAPE_H
#define _COLLISION_SHAPE_H

#include "math/vector2.h"
#include "math/box2d.h"

enum CollisionShapeType {
    CollisionShapeTypeCircle,
    CollisionShapeTypePolygon,
    CollisionShapeTypeCount,
};

struct ShapeOverlap {
    struct Vector2 normal;
    float depth;
};

struct CollisionShape {
    enum CollisionShapeType type;
};

typedef int (*CollisionFunction)(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap);

void collisionShapeBoundingBox(struct CollisionShape* shape, struct Vector2* at, struct Vector2* rotation, float scale, struct Box2D* outuput);
int collisionCollidePair(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap);

struct CollisionShape* collisionShapeUnpack(struct CollisionShape* shape, void* segmentRamStart);

#endif