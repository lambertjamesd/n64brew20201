#include "math/vector3.h"
#include "leveldefinition.h"
#include "levelbase.h"

unsigned nav_getClosestPoint(struct Vector3* closestTo, struct Vector3* allPoints, unsigned numPoints);
unsigned nav_getNextNavPoint(unsigned movingFrom, unsigned movingTo, char* nextNodeArr, unsigned numPoints);
unsigned getBasePathNodeID(struct PathfindingDefinition* inPathfinding, unsigned baseID);
float getDistanceToBase(struct PathfindingDefinition* inPathfinding, unsigned fromBase, unsigned toBase, unsigned numBases);

unsigned getClosestEnemyBase(struct PathfindingDefinition* inPathfinding, struct LevelBase* allBases, unsigned numBases, unsigned closestToBase, unsigned freindlyTeam);
unsigned getClosestNeutralBase(struct PathfindingDefinition* inPathfinding, struct LevelBase* allBases, unsigned numBases, unsigned closestToBase);