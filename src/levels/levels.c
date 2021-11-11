#include "levels.h"

#define DEFINE_LEVEL(humanName, name, theme, maxPlayers, flags)  extern char _##name##SegmentRomStart[], _##name##SegmentRomEnd[]; \
    extern struct LevelDefinition name##_Definition;
#include "level_list.h"
#undef DEFINE_LEVEL

#define DEFINE_THEME(name)  extern char _##name##SegmentRomStart[], _##name##SegmentRomEnd[];
#include "theme_list.h"
#undef DEFINE_THEME


#define DEFINE_THEME(name) struct ThemeMetadata g##name##ThemeMetadata = {_##name##SegmentRomStart, _##name##SegmentRomEnd, };
#include "theme_list.h"
#undef DEFINE_LEVEL

#define DEFINE_LEVEL(humanName, name, theme, maxPlayers, flags)  {humanName, &name##_Definition, _##name##SegmentRomStart, _##name##SegmentRomEnd, &g##theme##ThemeMetadata, maxPlayers, flags},

struct LevelMetadata gLevels[] = {
#include "level_list.h"
};

#undef DEFINE_LEVEL

unsigned gLevelCount = sizeof(gLevels) / sizeof(gLevels[0]);