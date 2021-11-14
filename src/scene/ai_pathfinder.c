#include "ai_pathfinder.h"

unsigned nav_getClosestPoint(struct Vector3* closestTo, struct Vector3* allPoints, unsigned numPoints){

    float minDist = vector3DistSqrd(&allPoints[0], closestTo);
    unsigned minIndex = 0;
    for(unsigned i = 1; i < numPoints; ++i){
        float thisDist = vector3DistSqrd(&allPoints[i], closestTo);
        if(thisDist < minDist){
            minIndex = i;
            minDist = thisDist;
        }
    }

    return minIndex;
}

unsigned nav_getNextNavPoint(unsigned movingFrom, unsigned movingTo, char* nextNodeArr, unsigned numPoints){
    return nextNodeArr[movingFrom * numPoints + movingTo];
}