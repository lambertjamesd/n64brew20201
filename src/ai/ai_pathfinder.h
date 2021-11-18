#include "math/vector3.h"
#include "scene/leveldefinition.h"
#include "scene/levelbase.h"

unsigned short nav_getClosestPoint(struct Vector3* closestTo, struct Vector3* allPoints, unsigned numPoints);
unsigned nav_getNextNavPoint(unsigned movingFrom, unsigned movingTo, char* nextNodeArr, unsigned numPoints);
unsigned getBasePathNodeID(struct PathfindingDefinition* inPathfinding, unsigned baseID);
float getDistanceToBase(struct PathfindingDefinition* inPathfinding, unsigned fromBase, unsigned toBase, unsigned numBases);

unsigned getClosestEnemyBase(struct PathfindingDefinition* inPathfinding, struct LevelBase* allBases, unsigned numBases, unsigned closestToBase, unsigned freindlyTeam);
unsigned getClosestNeutralBase(struct PathfindingDefinition* inPathfinding, struct LevelBase* allBases, unsigned numBases, unsigned closestToBase);

#define NODE_NONE   (~0)
#define NAV_ACCEPTANCE_RADIUS 100

struct Pathfinder {
    // the final position to goto after reaching targetNode
    struct Vector3 targetPosition;
    // the nearest node to targetPosition or NODE_NONE if it should move
    // directly towards targetPosition ignoring pathfinding
    unsigned short targetNode;
    // the next node to goto to ultimately reach targetNode or NODE_NONE
    // if the pathing agent should move towards targetPosition
    unsigned short currentNode;
};

// initializes pathfinder setting targetNode and currentNode to NODE_NONE
void pathfinderInit(struct Pathfinder* pathfinder, struct Vector3* startingPos);
// sets the new targetNode, targetPosition, and current node to path to current node
void pathfinderSetTarget(struct Pathfinder* pathfinder, struct PathfindingDefinition* pathfinding, struct Vector3* currentPosition, struct Vector3* target);
// updates the logic of the pathfinder, checking if it has reached currentNode 
// if so it should change currentNode to the next in the chain
// sets nextTarget to be the position of current node or targetPosition if
// there is no current node
void pathfinderUpdate(struct Pathfinder* pathfinder, struct PathfindingDefinition* pathfinding, struct Vector3* currentPosition, struct Vector3* nextTarget);
void pathfinderReset(struct Pathfinder* pathfinder);