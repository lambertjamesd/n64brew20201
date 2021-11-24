#include "ai_pathfinder.h"

unsigned getBaseFromNodeId(struct PathfindingDefinition* inPathfinding, unsigned nodeId, unsigned numBases){
    for(unsigned i = 0; i < numBases; ++i){
        if(inPathfinding->baseNodes[i] == nodeId) return i;
    }
    return -1;
}

unsigned short nav_getClosestPoint(struct Vector3* closestTo, struct Vector3* allPoints, unsigned numPoints){

    if (numPoints == 0) {
        return NODE_NONE;
    }

    float minDist = vector3DistSqrd(&allPoints[0], closestTo);
    unsigned short minIndex = NODE_NONE;
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
    if (movingFrom == NODE_NONE || movingTo == NODE_NONE) {
        return NODE_NONE;
    }
    return nextNodeArr[movingFrom * numPoints + movingTo];
}

unsigned getBasePathNodeID(struct PathfindingDefinition* inPathfinding, unsigned baseID){
    return inPathfinding->baseNodes[baseID];
}

float getDistanceToBase(struct PathfindingDefinition* inPathfinding, unsigned fromBase, unsigned toBase, unsigned numBases){
    unsigned from = getBasePathNodeID(inPathfinding, fromBase);
    unsigned to = getBasePathNodeID(inPathfinding, toBase);

    return (float)inPathfinding->nodeDistances[from * inPathfinding->nodeCount + to];
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

float pathfinderGetDistanceNextSegment(struct Pathfinder* pathfinder, struct PathfindingDefinition* pathfinding, struct Vector3* currentPosition) {
    unsigned next = nav_getNextNavPoint(pathfinder->currentNode, pathfinder->targetNode, pathfinding->nextNode, pathfinding->nodeCount);

    if (next == NODE_NONE) {
        return 0.0f;
    }

    struct Vector3 segmentPoint;

    if (next == pathfinder->currentNode) {
        segmentPoint = pathfinding->nodePositions[next];
    } else {
        struct Vector3 offset;
        vector3Sub(currentPosition, &pathfinding->nodePositions[pathfinder->currentNode], &offset);
        struct Vector3 edge;
        vector3Sub(&pathfinding->nodePositions[next], &pathfinding->nodePositions[pathfinder->currentNode], &edge);

        float lerp = vector3Dot(&offset, &edge) / vector3MagSqrd(&edge);

        lerp = MIN(lerp, 1.0f);
        lerp = MAX(lerp, 0.0f);

        vector3Lerp(&pathfinding->nodePositions[pathfinder->currentNode], &pathfinding->nodePositions[next], lerp, &segmentPoint);
    }

    return vector3DistSqrd(currentPosition, &segmentPoint);
}

void pathfinderUpdate(struct Pathfinder* pathfinder, struct PathfindingDefinition* pathfinding, struct Vector3* currentPosition, struct Vector3* nextTarget){
    if(pathfinder->currentNode != NODE_NONE){
        // get distance to next line segment so if ai is already
        // between the current node and the node after the current node
        // it can skip the current node and go straight to the next one
        unsigned int currDist = pathfinderGetDistanceNextSegment(pathfinder, pathfinding, currentPosition);
        if(currDist <= NAV_ACCEPTANCE_RADIUS*NAV_ACCEPTANCE_RADIUS){
            if(pathfinder->currentNode == pathfinder->targetNode){
                pathfinder->currentNode = NODE_NONE;
                pathfinder->targetNode = NODE_NONE;
            }
            else {
                pathfinder->currentNode = nav_getNextNavPoint(pathfinder->currentNode, pathfinder->targetNode, pathfinding->nextNode, pathfinding->nodeCount);
            }
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

void pathfinderReset(struct Pathfinder* pathfinder){
    pathfinder->targetNode = NODE_NONE;
    pathfinder->currentNode = NODE_NONE;
}