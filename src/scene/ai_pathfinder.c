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

unsigned getBasePathNodeID(struct PathfindingDefinition* inPathfinding, unsigned baseID){
    return inPathfinding->baseNodes[baseID];
}

float getDistanceToBase(struct PathfindingDefinition* inPathfinding, unsigned fromBase, unsigned toBase, unsigned numBases){
    unsigned from = getBasePathNodeID(inPathfinding, fromBase);
    unsigned to = getBasePathNodeID(inPathfinding, toBase);

    for(unsigned i = 0; i < numBases; ++i){
        if(inPathfinding->baseDistances[i].fromBase == from &&
        inPathfinding->baseDistances[i].toBase == to){
            return inPathfinding->baseDistances[i].distance;
        }
    }
    return -1.f;
}

unsigned getClosestEnemyBase(struct PathfindingDefinition* inPathfinding, struct LevelBase* allBases, unsigned numBases, unsigned closestToBase, unsigned freindlyTeam){
    unsigned outInd = -1;
    float minDist = ~0;

    for(unsigned i = 0; i < numBases; ++i){
        if(allBases[i].team.teamNumber != freindlyTeam){
            if(outInd == -1){
                outInd = i;
                minDist = getDistanceToBase(inPathfinding, closestToBase, i, numBases);
            }
            else{
                float thisDist = getDistanceToBase(inPathfinding, closestToBase, i, numBases);
                if(minDist < thisDist){
                    outInd = i;
                    minDist = thisDist;
                }
            }
        }
    }

    return outInd;
}

unsigned getClosestNeutralBase(struct PathfindingDefinition* inPathfinding, struct LevelBase* allBases, unsigned numBases, unsigned closestToBase){
    unsigned outInd = -1;
    float minDist = ~0;

    for(unsigned i = 0; i < numBases; ++i){
        if(allBases[i].team.teamNumber == TEAM_NONE){
            if(outInd == -1){
                outInd = i;
                minDist = getDistanceToBase(inPathfinding, closestToBase, i, numBases);
            }
            else{
                float thisDist = getDistanceToBase(inPathfinding, closestToBase, i, numBases);
                if(minDist < thisDist){
                    outInd = i;
                    minDist = thisDist;
                }
            }
        }
    }

    return outInd;
}