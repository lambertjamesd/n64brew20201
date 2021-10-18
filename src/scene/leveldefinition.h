#ifndef _LEVEL_DEFINITION_H
#define _LEVEL_DEFINITION_H

#include <ultra64.h>
#include "math/vector2.h"

#define STARTING_FACTION_NONE   4

struct BaseDefinition {
    struct Vector2 position;
    unsigned char startingFaction;
};

struct LevelDefinition {
    unsigned int baseCount;
    struct BaseDefinition* bases;
    Gfx* sceneRender;
};

#endif