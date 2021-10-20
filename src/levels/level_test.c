
#include "level_test.h"
#include "../data/levels/test/test.h"
#include "game_defs.h"

struct BaseDefinition gLevelTestBases[] = {
    {
        .position = {0 * SCENE_SCALE, 0 * SCENE_SCALE},
        .startingTeam = TEAM(0),
    },
    {
        .position = {10.0f * SCENE_SCALE, 0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {20.0f * SCENE_SCALE, 0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {0 * SCENE_SCALE, 10.0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {10.0f * SCENE_SCALE, 10.0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {20.0f * SCENE_SCALE, 10.0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {0 * SCENE_SCALE, 20.0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {10.0f * SCENE_SCALE, 20.0 * SCENE_SCALE},
        .startingTeam = TEAM_NONE,
    },
    {
        .position = {20.0f * SCENE_SCALE, 20.0 * SCENE_SCALE},
        .startingTeam = TEAM(1),
    },
};

struct Vector2 gLevelTestPlayerStart[] = {
    {0 * SCENE_SCALE, 0 * SCENE_SCALE},
    {20.0f * SCENE_SCALE, 20.0f * SCENE_SCALE},
};

struct LevelDefinition gLevelTest = {
    .maxPlayerCount = 2,
    .playerStartLocations = gLevelTestPlayerStart,
    .baseCount = sizeof(gLevelTestBases) / sizeof(gLevelTestBases[0]),
    .bases = gLevelTestBases,
    .sceneRender = test_level_geometry,
};