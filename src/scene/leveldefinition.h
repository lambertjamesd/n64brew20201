#ifndef _LEVEL_DEFINITION_H
#define _LEVEL_DEFINITION_H

#include <ultra64.h>
#include "math/vector2.h"
#include "math/box2d.h"
#include "math/quaternion.h"
#include "collision/staticscene.h"

#define TEAM(index) (index)
#define TEAM_NONE   4

struct ThemeDefinition;

struct BaseDefinition {
    struct Vector2 position;
    unsigned char startingTeam;
};

struct DecorDefinition {
    struct Vector3 position;
    struct Quaternion rotation;
    unsigned decorID;
};

struct LevelDefinition {
    unsigned int maxPlayerCount;
    struct Vector2* playerStartLocations;
    unsigned short baseCount;
    unsigned short decorCount;
    struct BaseDefinition* bases;
    struct DecorDefinition* decor;
    struct Box2D levelBoundaries;
    Gfx* sceneRender;
    struct ThemeDefinition* theme;
    struct StaticScene staticScene;
};

struct LevelDefinition* levelDefinitionUnpack(struct LevelDefinition* addressPtr, void* segmentRamStart, void* themeSegmentStart);

#endif