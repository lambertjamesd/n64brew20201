#include "leveldefinition.h"
#include "util/rom.h"

struct LevelDefinition* levelDefinitionUnpack(struct LevelDefinition* addressPtr, void* segmentRamStart) {
    struct LevelDefinition* result = CALC_RAM_POINTER(addressPtr, segmentRamStart);
    result->playerStartLocations = CALC_RAM_POINTER(result->playerStartLocations, segmentRamStart);
    result->bases = CALC_RAM_POINTER(result->bases, segmentRamStart);
    // result->staticScene.boundary = CALC_RAM_POINTER(result->staticScene.boundary, segmentRamStart);
    return result;
}