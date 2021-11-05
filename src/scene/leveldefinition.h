#ifndef _LEVEL_DEFINITION_H
#define _LEVEL_DEFINITION_H

#include <ultra64.h>
#include "math/vector2.h"
#include "math/box2d.h"
#include "collision/staticscene.h"

#define TEAM(index) (index)
#define TEAM_NONE   4

struct BaseDefinition {
    struct Vector2 position;
    unsigned char startingTeam;
};

struct DecorDefinition {
    struct Vector2 position;
    float rotation;
    unsigned decorID;
};

struct LevelDefinition {
    unsigned int maxPlayerCount;
    struct Vector2* playerStartLocations;
    unsigned short baseCount;
    unsigned short decorCount;
    struct BaseDefinition* bases;
    struct DecorDefinition decor;
    struct Box2D levelBoundaries;
    Gfx* sceneRender;
    struct StaticScene staticScene;
};

struct LevelDefinition* levelDefinitionUnpack(struct LevelDefinition* addressPtr, void* segmentRamStart);

#endif