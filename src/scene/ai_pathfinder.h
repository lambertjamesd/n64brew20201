#include "math/vector3.h"
#include "leveldefinition.h"

unsigned nav_getClosestPoint(struct Vector3* closestTo, struct Vector3* allPoints, unsigned numPoints);
unsigned nav_getNextNavPoint(unsigned movingFrom, unsigned movingTo, char* nextNodeArr, unsigned numPoints);