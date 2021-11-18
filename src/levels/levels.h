#ifndef _LEVELS_H
#define _LEVELS_H

#include <ultra64.h>
#include "scene/leveldefinition.h"

struct ThemeMetadata {
    char* romSegmentStart;
    char* romSegmentEnd;
};

struct WireframeMetadata {
    char* romSegmentStart;
    char* romSegmentEnd;
    Gfx* wireframe;
};

struct LevelMetadata {
    char* name;
    struct LevelDefinition* fullDefinition;
    char* romSegmentStart;
    char* romSegmentEnd;
    struct ThemeMetadata* theme;
    struct WireframeMetadata wireframe;
    unsigned char maxPlayers;
    unsigned short flags;
};

extern struct LevelMetadata gLevels[];
extern unsigned gLevelCount;

#endif