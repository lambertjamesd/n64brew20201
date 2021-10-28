#ifndef _LEVELS_H
#define _LEVELS_H

#include "scene/leveldefinition.h"

struct LevelMetadata {
    struct LevelDefinition* fullDefinition;
    char* romSegmentStart;
    char* romSegmentEnd;
};

extern struct LevelMetadata gLevels[];
extern unsigned gLevelCount;

#endif