#ifndef _LEVELS_THEME_DEFINITON_H
#define _LEVELS_THEME_DEFINITON_H

#include <ultra64.h>
#include "collision/shape.h"

struct ThemeDefinition {
    Gfx** decorMaterials;
    Gfx** decorDisplayLists;
    struct CollisionShape** decorShapes;
};

struct ThemeDefinition* levelDefinitionUnpack(struct ThemeDefinition* addressPtr, void* segmentRamStart);

#endif