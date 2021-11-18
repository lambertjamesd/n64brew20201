#include "ai_pathfinder.h"

unsigned short nav_getClosestPoint(struct Vector3* closestTo, struct Vector3* allPoints, unsigned numPoints){

    float minDist = vector3DistSqrd(&allPoints[0], closestTo);
    unsigned short minIndex = 0;
    for(unsigned short i = 1; i < numPoints; ++i){
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

void pathfinderInit(struct Pathfinder* pathfinder, struct Vector3* startingPos){
    pathfinder->currentNode = NODE_NONE;
    pathfinder->targetNode = NODE_NONE;
}

void pathfinderUpdate(struct Pathfinder* pathfinder, struct PathfindingDefinition* pathfinding, struct Vector3* currentPosition, struct Vector3* nextTarget){
    if(pathfinder->currentNode != NODE_NONE){
        unsigned int currDist = vector3DistSqrd(currentPosition, &pathfinding->nodePositions[pathfinder->currentNode]);
        if(currDist <= NAV_ACCEPTANCE_RADIUS*NAV_ACCEPTANCE_RADIUS){
            if(pathfinder->currentNode == pathfinder->targetNode){
                pathfinder->currentNode = NODE_NONE;
                pathfinder->targetNode = NODE_NONE;
            }
            else pathfinder->currentNode = nav_getNextNavPoint(pathfinder->currentNode, pathfinder->targetNode, pathfinding->nextNode, pathfinding->nodeCount);
        }
    }
}

void pathfinderSetTarget(struct Pathfinder* pathfinder, struct PathfindingDefinition* pathfinding, struct Vector3* currentPosition, struct Vector3* target){
    unsigned short from = nav_getClosestPoint(currentPosition, pathfinding->nodePositions, pathfinding->nodeCount);
    unsigned short to = nav_getClosestPoint(target, pathfinding->nodePositions, pathfinding->nodeCount);
    pathfinder->targetNode = to;
    pathfinder->currentNode = from;
    
    pathfinder->targetPosition.x = target->x;
    pathfinder->targetPosition.y = target->y;
    pathfinder->targetPosition.z = target->z;
}