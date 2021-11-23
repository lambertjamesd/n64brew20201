#include "leveldefinition.h"
#include "util/rom.h"
#include "levels/themedefinition.h"

struct LevelDefinition* levelDefinitionUnpack(struct LevelDefinition* addressPtr, void* segmentRamStart, void* themeSegmentStart) {
    struct LevelDefinition* result = CALC_RAM_POINTER(addressPtr, segmentRamStart);
    result->playerStartLocations = CALC_RAM_POINTER(result->playerStartLocations, segmentRamStart);
    result->bases = CALC_RAM_POINTER(result->bases, segmentRamStart);
    result->decor = CALC_RAM_POINTER(result->decor, segmentRamStart);
    result->staticScene.boundary = CALC_RAM_POINTER(result->staticScene.boundary, segmentRamStart);
    result->theme = levelThemeUnpack(result->theme, themeSegmentStart);
    result->pathfinding.nodePositions = CALC_RAM_POINTER(result->pathfinding.nodePositions, segmentRamStart);
    result->pathfinding.nextNode = CALC_RAM_POINTER(result->pathfinding.nextNode, segmentRamStart);
    result->pathfinding.baseNodes = CALC_RAM_POINTER(result->pathfinding.baseNodes, segmentRamStart);
    result->pathfinding.nodeDistances = CALC_RAM_POINTER(result->pathfinding.nodeDistances, segmentRamStart);
    return result;
}