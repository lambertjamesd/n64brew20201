#include "level.h"
#include "../../level_themes/Space/theme.h"
#include "sk64/skelatool_defs.h"
#include <ultra64.h>

Vtx Boot_Camp_Plane_003_color[] = {
    {{{-4902, 833, 3482}, 0, {-11362, -12}, {195, 235, 251, 255}}},
    {{{-4902, 0, 3482}, 0, {-11362, 2060}, {195, 235, 251, 255}}},
    {{{-4916, 0, 1082}, 0, {565, 2060}, {195, 235, 251, 255}}},
    {{{-4916, 833, 1082}, 0, {565, -12}, {195, 235, 251, 255}}},
    {{{-2281, 0, 1082}, 0, {565, 2060}, {195, 235, 251, 255}}},
    {{{-2281, 0, -1081}, 0, {11320, 2060}, {195, 235, 251, 255}}},
    {{{-2281, 833, -1081}, 0, {11320, -12}, {195, 235, 251, 255}}},
    {{{-2281, 833, 1082}, 0, {565, -12}, {195, 235, 251, 255}}},
    {{{3681, 0, -1084}, 0, {11333, 2060}, {198, 236, 251, 255}}},
    {{{3681, 0, 1081}, 0, {567, 2060}, {195, 235, 251, 255}}},
    {{{3681, 833, 1081}, 0, {567, -12}, {195, 235, 251, 255}}},
    {{{3681, 833, -1084}, 0, {11333, -12}, {195, 235, 251, 255}}},
    {{{3681, 0, 1081}, 0, {8860, 2060}, {195, 235, 251, 255}}},
    {{{399, 0, 1081}, 0, {-7450, 2060}, {195, 235, 251, 255}}},
    {{{399, 833, 1081}, 0, {-7450, -12}, {195, 235, 251, 255}}},
    {{{3681, 833, 1081}, 0, {8860, -12}, {195, 235, 251, 255}}},
    {{{-2281, 833, -1081}, 0, {-13611, -12}, {195, 235, 251, 255}}},
    {{{-2281, 0, -1081}, 0, {-13611, 2060}, {195, 235, 251, 255}}},
    {{{3681, 0, -1084}, 0, {16028, 2060}, {198, 236, 251, 255}}},
    {{{3681, 833, -1084}, 0, {16028, -12}, {195, 235, 251, 255}}},
    {{{413, 833, 3481}, 0, {-11361, -12}, {195, 235, 251, 255}}},
    {{{399, 833, 1081}, 0, {566, -12}, {195, 235, 251, 255}}},
    {{{399, 0, 1081}, 0, {566, 2060}, {195, 235, 251, 255}}},
    {{{413, 0, 3481}, 0, {-11361, 2060}, {195, 235, 251, 255}}},
    {{{-4916, 0, 1082}, 0, {-6736, 2060}, {195, 235, 251, 255}}},
    {{{-2281, 0, 1082}, 0, {6356, 2060}, {195, 235, 251, 255}}},
    {{{-2281, 833, 1082}, 0, {6356, -12}, {195, 235, 251, 255}}},
    {{{-4916, 833, 1082}, 0, {-6736, -12}, {195, 235, 251, 255}}},
    {{{-4902, 0, 3482}, 0, {-12172, 2060}, {195, 235, 251, 255}}},
    {{{-4902, 833, 3482}, 0, {-12172, -12}, {195, 235, 251, 255}}},
    {{{413, 833, 3481}, 0, {14249, -12}, {195, 235, 251, 255}}},
    {{{413, 0, 3481}, 0, {14249, 2060}, {195, 235, 251, 255}}},
};


Vtx Boot_Camp_GeometryGround_color[] = {
    {{{3681, 0, 1081}, 0, {6548, 894}, {255, 255, 255, 255}}},
    {{{3681, 0, -1084}, 0, {6548, -1869}, {255, 255, 255, 255}}},
    {{{-2281, 0, -1081}, 0, {-1062, -1866}, {255, 255, 255, 255}}},
    {{{399, 0, 1081}, 0, {2360, 894}, {255, 255, 255, 255}}},
    {{{-2281, 0, 1082}, 0, {-1062, 894}, {255, 255, 255, 255}}},
    {{{413, 0, 3481}, 0, {2378, 3957}, {255, 255, 255, 255}}},
    {{{-4916, 0, 1082}, 0, {-4424, 894}, {255, 255, 255, 255}}},
    {{{-4902, 0, 3482}, 0, {-4406, 3957}, {255, 255, 255, 255}}},
};


Gfx Boot_Camp_model_gfx[] = {
    // Material SSWall
    gsSPDisplayList(Space_SSWall),
    // End Material SSWall
    gsSPVertex(&Boot_Camp_Plane_003_color[0], 32, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(4, 5, 6, 0, 4, 6, 7, 0),
    gsSP2Triangles(8, 9, 10, 0, 8, 10, 11, 0),
    gsSP2Triangles(12, 13, 14, 0, 12, 14, 15, 0),
    gsSP2Triangles(16, 17, 18, 0, 16, 18, 19, 0),
    gsSP2Triangles(20, 21, 22, 0, 20, 22, 23, 0),
    gsSP2Triangles(24, 25, 26, 0, 24, 26, 27, 0),
    gsSP2Triangles(28, 29, 30, 0, 28, 30, 31, 0),
    // Material SSFloor
    gsSPDisplayList(Space_SSFloor),
    // End Material SSFloor
    gsSPVertex(&Boot_Camp_GeometryGround_color[0], 8, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(3, 2, 4, 0, 3, 4, 5, 0),
    gsSP2Triangles(4, 6, 7, 0, 4, 7, 5, 0),
    // Begin decor
    gsSPDisplayList(Space_Lockers),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 4, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_LockersDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPDisplayList(Space_SSCrate),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 2, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_CrateDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 9, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_CrateDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 10, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_CrateDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPDisplayList(Space_SetTextures2),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 0, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_ChairDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 6, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_ChairDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 7, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_ChairDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 1, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_ConsoleDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 8, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_ConsoleDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 3, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_Food_DishesDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 5, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_TableDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPEndDisplayList(),
};
struct BaseDefinition Boot_Camp_Bases[] = {
    {.position = {2700, 0}, .startingTeam = TEAM(1)},
    {.position = {100, 0}, .startingTeam = TEAM_NONE},
    {.position = {-1283.78, 2568.44}, .startingTeam = TEAM_NONE},
};

struct Vector2 Boot_Camp_StartingPositions[] = {
    {-3753.75, 2324.36},
    {2700, 0},
};

struct SceneBoundary Boot_Camp_Boundary[] = {
    {{-4902.41, 3482.02}, {0.999983, -0.00573945}, 2400.04},
    {{-4916.18, 1082.02}, {-0, 1}, 2634.34},
    {{-2281.84, 1082.02}, {1, 0}, 2163.86},
    {{-2281.84, -1081.84}, {0.000421721, 1}, 5963.69},
    {{3681.84, -1084.36}, {-1, 0}, 2166.15},
    {{3681.84, 1081.79}, {-3.47035e-05, -1}, 3281.84},
    {{400, 1081.9}, {-0.999983, 0.00573964}, 2400.04},
    {{413.775, 3481.9}, {-2.14006e-05, -1}, 5316.18},
};

struct DecorDefinition Boot_Camp_Decor[] = {
    {{-3131.26, 0, 1377.67}, {0, 0, 0, 1}, 1, CHAIR_DECOR_ID},
    {{1009.59, 0, -788.503}, {0, -1.19209e-07, 0, 1}, 1, CONSOLE_DECOR_ID},
    {{-4636.77, 0, 3317.62}, {0, 0, 0, 1}, 1, CRATE_DECOR_ID},
    {{-1776.04, 0, -800.104}, {0, 0, 0, 1}, 1, FOOD_DISHES_DECOR_ID},
    {{310.556, 0, 2551.34}, {2.98023e-08, -0.707107, 8.9407e-08, 0.707107}, 1, LOCKERS_DECOR_ID},
    {{-3569.91, 0, 1632}, {2.98023e-08, -0.707107, 8.9407e-08, 0.707107}, 1, TABLE_DECOR_ID},
    {{-3577.6, 0, 1377.67}, {0, 0, 0, 1}, 1, CHAIR_DECOR_ID},
    {{-3936.43, 0, 1377.67}, {0, 0, 0, 1}, 1, CHAIR_DECOR_ID},
    {{1622.23, 0, -788.503}, {0, -1.19209e-07, 0, 1}, 1, CONSOLE_DECOR_ID},
    {{-4540.49, 0, 3142.58}, {0, 0, 0, 1}, 1, CRATE_DECOR_ID},
    {{-4444.22, 0, 3291.37}, {0, 0, 0, 1}, 1, CRATE_DECOR_ID},
};
struct Vector3 Boot_Camp_PathingNodes[] = {
    {-1281.44, 0.0f, 2559.5},
    {78.5527, 0.0f, -11.5671},
    {2715.16, 0.0f, 4.81832},
};
char Boot_Camp_BasesPathNodes[] = {
    2,
    1,
    0,
};
unsigned short Boot_Camp_NodeDistances[] = {
    0, 2908, 5545, 
    2908, 0, 2636, 
    5545, 2636, 0, 
};
char Boot_Camp_NextNode[] = {
    0, 1, 1, 
    0, 1, 2, 
    1, 1, 2, 
};
struct LevelDefinition Boot_Camp_Definition = {
    .maxPlayerCount = 2,
    .playerStartLocations = Boot_Camp_StartingPositions,
    .baseCount = 3,
    .decorCount = 11,
    .bases = Boot_Camp_Bases,
    .decor = Boot_Camp_Decor,
    .levelBoundaries = {{-4916.18, -1084.36}, {3681.84, 3482.02}},
    .sceneRender = Boot_Camp_model_gfx,
    .theme = &SpaceTheme,
    .staticScene = {Boot_Camp_Boundary, 8},
    .pathfinding = {.nodeCount = 3, .baseNodes = Boot_Camp_BasesPathNodes, .nodeDistances = Boot_Camp_NodeDistances, .nodePositions = Boot_Camp_PathingNodes, .nextNode = Boot_Camp_NextNode},
    .aiDifficulty = 0,
};

