#ifndef _LEVELS_THEME_DEFINITON_H
#define _LEVELS_THEME_DEFINITON_H

#include <ultra64.h>
#include "collision/shape.h"
#include "collision/polygon.h"
#include "collision/circle.h"

struct ThemeDefinition {
    Gfx* skybox;
    Gfx* skyboxMaterial;
    Gfx** decorMaterials;
    Gfx** decorDisplayLists;
    struct CollisionShape** decorShapes;
    unsigned short decorCount;
};

struct ThemeDefinition* levelThemeUnpack(struct ThemeDefinition* addressPtr, void* segmentRamStart);

#endif