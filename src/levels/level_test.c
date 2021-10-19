
#include "level_test.h"
#include "../data/levels/test/test.h"
#include "game_defs.h"

struct BaseDefinition gLevelTestBases[] = {
    {
        .position = {0 * SCENE_SCALE, 0 * SCENE_SCALE},
        .startingFaction = FACTION(0),
    }
};

struct LevelDefinition gLevelTest = {
    .baseCount = 1,
    .bases = gLevelTestBases,
    .sceneRender = test_level_geometry,
};