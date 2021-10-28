#ifndef _LEVEL_DEFINITION_H
#define _LEVEL_DEFINITION_H

#include <ultra64.h>
#include "math/vector2.h"

#define TEAM(index) (index)
#define TEAM_NONE   4

struct BaseDefinition {
    struct Vector2 position;
    unsigned char startingTeam;
};

struct LevelDefinition {
    unsigned int maxPlayerCount;
    struct Vector2* playerStartLocations;
    unsigned int baseCount;
    struct BaseDefinition* bases;
    Gfx* sceneRender;
};

struct LevelDefinition* levelDefinitionUnpack(struct LevelDefinition* addressPtr, void* segmentRamStart);

#endif