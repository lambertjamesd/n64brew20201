#include "themedefinition.h"
#include "util/rom.h"

struct ThemeDefinition* levelThemeUnpack(struct ThemeDefinition* addressPtr, void* segmentRamStart) {
    if (!addressPtr) {
        return 0;
    }

    struct ThemeDefinition* result = CALC_RAM_POINTER(addressPtr, segmentRamStart);
    result->decorMaterials = CALC_RAM_POINTER(result->decorMaterials, segmentRamStart);
    result->decorDisplayLists = CALC_RAM_POINTER(result->decorDisplayLists, segmentRamStart);
    result->decorShapes = CALC_RAM_POINTER(result->decorShapes, segmentRamStart);

    for (unsigned i = 0; i < result->decorCount; ++i) {
        result->decorShapes[i] = collisionShapeUnpack(result->decorShapes[i], segmentRamStart);
    }

    return result;
}