#ifndef _COLLISION_POLYGON_H
#define _COLLISION_POLYGON_H

#include "shape.h"
#include "math/vector2.h"

struct CollisionPolygonEdge {
    struct Vector2 corner;
    struct Vector2 normal;
    float edgeLen;
};

struct CollisionPolygon {
    struct CollisionShape shapeCommon;
    struct CollisionPolygonEdge* edges;
    unsigned edgeCount;
};

void collisionPolygonBoundingBox(struct CollisionPolygon* shape, struct Vector2* at, struct Vector2* rotation, float scale, struct Box2D* outuput);

int collisionCirclePolygon(struct CollisionShape* a, struct CollisionShape* b, struct Vector2* aToB, struct ShapeOverlap* shapeOverlap);

#endif