#ifndef _LEVELS_H
#define _LEVELS_H

#include "scene/leveldefinition.h"

struct ThemeMetadata {
    char* romSegmentStart;
    char* romSegmentEnd;
};

struct LevelMetadata {
    char* name;
    struct LevelDefinition* fullDefinition;
    char* romSegmentStart;
    char* romSegmentEnd;
    struct ThemeMetadata* theme;
    unsigned char maxPlayers;
    unsigned short flags;
};

extern struct LevelMetadata gLevels[];
extern unsigned gLevelCount;

#endif