#include "level.h"
#include "../../level_themes/Mars/theme.h"
#include "sk64/skelatool_defs.h"
#include <ultra64.h>

Vtx Level_Template_Grid_001_color[] = {
    {{{-4007, 0, 0}, 0, {-763, 8}, {203, 98, 97, 255}}},
    {{{-4783, 524, -2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{-4783, 524, 4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{-4007, 0, 4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{-4783, 524, 4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{-4783, 524, -2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{-5780, 524, 12}, 0, {-761, 991}, {203, 98, 97, 255}}},
    {{{-5780, 524, 5360}, 0, {735, 1012}, {202, 93, 92, 255}}},
    {{{2, 524, 5360}, 0, {2002, 996}, {202, 92, 91, 255}}},
    {{{0, 524, 4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{-4007, 0, 4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{-4783, 524, 4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{0, 524, 4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{0, 0, 4007}, 0, {1999, -16}, {202, 92, 91, 255}}},
    {{{4007, 0, 0}, 0, {-763, 8}, {203, 98, 97, 255}}},
    {{{4007, 0, 4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{4783, 524, 4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{4783, 524, -2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{4783, 524, 4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{0, 524, 4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{-2, 524, 5360}, 0, {2002, 996}, {202, 92, 91, 255}}},
    {{{5780, 524, 5360}, 0, {735, 1012}, {202, 93, 92, 255}}},
    {{{5780, 524, 12}, 0, {-761, 991}, {203, 98, 97, 255}}},
    {{{4783, 524, -2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{4007, 0, 4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{0, 524, 4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{4783, 524, 4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{-4007, 0, -4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{-4783, 524, -4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{-4783, 524, 2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{-4783, 524, -4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{0, 524, -4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{2, 524, -5360}, 0, {2002, 996}, {202, 92, 91, 255}}},
    {{{-5780, 524, -5360}, 0, {735, 1012}, {202, 93, 92, 255}}},
    {{{-5780, 524, -12}, 0, {-761, 991}, {203, 98, 97, 255}}},
    {{{-4783, 524, 2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{-4007, 0, -4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{0, 0, -4007}, 0, {1999, -16}, {202, 92, 91, 255}}},
    {{{0, 524, -4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{-4783, 524, -4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{4783, 524, 2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{4783, 524, -4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{4007, 0, -4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{4783, 524, -4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{4783, 524, 2}, 0, {-764, 542}, {203, 98, 97, 255}}},
    {{{5780, 524, -12}, 0, {-761, 991}, {203, 98, 97, 255}}},
    {{{5780, 524, -5360}, 0, {735, 1012}, {202, 93, 92, 255}}},
    {{{-2, 524, -5360}, 0, {2002, 996}, {202, 92, 91, 255}}},
    {{{0, 524, -4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
    {{{4007, 0, -4007}, 0, {358, 5}, {202, 93, 92, 255}}},
    {{{4783, 524, -4662}, 0, {559, 555}, {202, 93, 92, 255}}},
    {{{0, 524, -4662}, 0, {2000, 536}, {202, 92, 91, 255}}},
};


Vtx Level_Template_Grid_color[] = {
    {{{4007, 0, -4007}, 0, {-1806, 2830}, {202, 93, 92, 255}}},
    {{{-4007, 0, -4007}, 0, {2830, 2830}, {202, 93, 92, 255}}},
    {{{-4007, 0, 4007}, 0, {2830, -1806}, {202, 93, 92, 255}}},
    {{{4007, 0, 4007}, 0, {-1806, -1806}, {202, 93, 92, 255}}},
};


Gfx Level_Template_model_gfx[] = {
    // Material Rock
    gsSPDisplayList(Mars_Rock),
    // End Material Rock
    gsSPVertex(&Level_Template_Grid_001_color[0], 30, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(4, 5, 6, 0, 7, 4, 6, 0),
    gsSP2Triangles(4, 7, 8, 0, 9, 4, 8, 0),
    gsSP2Triangles(10, 11, 12, 0, 10, 12, 13, 0),
    gsSP2Triangles(14, 15, 16, 0, 14, 16, 17, 0),
    gsSP2Triangles(18, 19, 20, 0, 21, 18, 20, 0),
    gsSP2Triangles(18, 21, 22, 0, 23, 18, 22, 0),
    gsSP2Triangles(24, 13, 25, 0, 24, 25, 26, 0),
    gsSP2Triangles(0, 27, 28, 0, 0, 28, 29, 0),
    gsSPVertex(&Level_Template_Grid_001_color[14], 1, 0),
    gsSPVertex(&Level_Template_Grid_001_color[30], 22, 1),
    gsSP2Triangles(1, 2, 3, 0, 4, 1, 3, 0),
    gsSP2Triangles(1, 4, 5, 0, 6, 1, 5, 0),
    gsSP2Triangles(7, 8, 9, 0, 7, 9, 10, 0),
    gsSP2Triangles(0, 11, 12, 0, 0, 12, 13, 0),
    gsSP2Triangles(14, 15, 16, 0, 17, 14, 16, 0),
    gsSP2Triangles(14, 17, 18, 0, 19, 14, 18, 0),
    gsSP2Triangles(20, 21, 22, 0, 20, 22, 8, 0),
    // Material Ground
    gsSPDisplayList(Mars_Ground),
    // End Material Ground
    gsSPVertex(&Level_Template_Grid_color[0], 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    // Begin decor
    gsSPDisplayList(Mars_Crate),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 5, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_CrateDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPDisplayList(Mars_Metal),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 9, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_Broken_BaseDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 4, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_ConsoleBDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 8, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_DishDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 7, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_DishBaseDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 6, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_SearchlightDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPDisplayList(Mars_Rock),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 2, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_Rock001DisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 1, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_Rock002DisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 3, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_Rock003DisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 0, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Mars_Rock004DisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPEndDisplayList(),
};
struct BaseDefinition Level_Template_Bases[] = {
    {.position = {0, 700}, .startingTeam = TEAM(0)},
    {.position = {0, 0}, .startingTeam = TEAM(1)},
    {.position = {0, 1400}, .startingTeam = TEAM(2)},
    {.position = {0, 2100}, .startingTeam = TEAM(3)},
    {.position = {800, 0}, .startingTeam = TEAM_NONE},
};

struct Vector2 Level_Template_StartingPositions[] = {
    {0, 700},
    {0, 0},
    {0, 1400},
    {0, 2100},
};

struct SceneBoundary Level_Template_Boundary[] = {
    {{0, -4046.9}, {-0, 1}},
    {{4046.9, 0}, {-1, 0}},
    {{0, 4046.9}, {-0, -1}},
    {{-4046.9, 0}, {1, 0}},
};

struct DecorDefinition Level_Template_Decor[] = {
    {{-3000, 0, 1693.43}, {0, 0, 0, 1}, ROCK004_DECOR_ID},
    {{-3000, 0, -1300}, {0, 0, 0, 1}, ROCK002_DECOR_ID},
    {{-3000, 0, -3000}, {0, 0, 0, 1}, ROCK001_DECOR_ID},
    {{-3000, 0, 200}, {0, 0, 0, 1}, ROCK003_DECOR_ID},
    {{-1924.5, 0, -3375.11}, {0, 0, 0, 1}, CONSOLEB_DECOR_ID},
    {{-1500, 0, -3600}, {0, 0, 0, 1}, CRATE_DECOR_ID},
    {{-1500, 0, -3200}, {8.9407e-08, 0.184693, -1.49012e-08, 0.982796}, SEARCHLIGHT_DECOR_ID},
    {{-1851.11, 0, -1730.78}, {0, 0.996393, 0, 0.0848587}, DISHBASE_DECOR_ID},
    {{-146.485, 136.501, -1530.49}, {0.223327, 0.839707, -0.407277, 0.281324}, DISH_DECOR_ID},
    {{-48.5003, 0, -3712.41}, {0, 0, 0, 1}, BROKEN_BASE_DECOR_ID},
};
struct Vector3 Level_Template_PathingNodes[] = {
};
char Level_Template_NextNode[] = {
};
struct LevelDefinition Level_Template_Definition = {
    .maxPlayerCount = 4,
    .playerStartLocations = Level_Template_StartingPositions,
    .baseCount = 5,
    .decorCount = 10,
    .bases = Level_Template_Bases,
    .decor = Level_Template_Decor,
    .levelBoundaries = {{-4046.9, -4046.9}, {4046.9, 4046.9}},
    .sceneRender = Level_Template_model_gfx,
    .theme = &MarsTheme,
    .staticScene = {Level_Template_Boundary, 4},
    .pathfinding = {.nodeCount = 0, .nodePositions = Level_Template_PathingNodes, .nextNode = Level_Template_NextNode},
};

