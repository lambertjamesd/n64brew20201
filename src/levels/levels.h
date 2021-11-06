#ifndef _LEVELS_H
#define _LEVELS_H

#include "scene/leveldefinition.h"

struct ThemeMetadata {
    char* romSegmentStart;
    char* romSegmentEnd;
};

struct LevelMetadata {
    struct LevelDefinition* fullDefinition;
    char* romSegmentStart;
    char* romSegmentEnd;
    struct ThemeMetadata* theme;
};

extern struct LevelMetadata gLevels[];
extern unsigned gLevelCount;

#endif