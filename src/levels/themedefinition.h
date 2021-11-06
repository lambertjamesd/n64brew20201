#ifndef _LEVELS_THEME_DEFINITON_H
#define _LEVELS_THEME_DEFINITON_H

#include <ultra64.h>
#include "collision/shape.h"
#include "collision/polygon.h"

struct ThemeDefinition {
    Gfx** decorMaterials;
    Gfx** decorDisplayLists;
    struct CollisionShape** decorShapes;
    unsigned short decorCount;
};

struct ThemeDefinition* levelThemeUnpack(struct ThemeDefinition* addressPtr, void* segmentRamStart);

#endif