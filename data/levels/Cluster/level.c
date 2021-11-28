#include "level.h"
#include "../../level_themes/Mars/theme.h"
#include "sk64/skelatool_defs.h"
#include <ultra64.h>

Vtx Cluster_Grid_001_color[] = {
    {{{-4007, 0, 0}, 0, {-1526, 16}, {203, 98, 97, 255}}},
    {{{-4783, 524, -2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{-4783, 524, 4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{-4007, 0, 4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{-4783, 524, 4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{-4783, 524, -2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{-5780, 524, 12}, 0, {-1523, 1983}, {203, 98, 97, 255}}},
    {{{-5780, 524, 5360}, 0, {1470, 2024}, {202, 93, 92, 255}}},
    {{{2, 524, 5360}, 0, {4004, 1992}, {202, 92, 91, 255}}},
    {{{0, 524, 4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{-4007, 0, 4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{-4783, 524, 4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{0, 524, 4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{0, 0, 4007}, 0, {3999, -32}, {202, 92, 91, 255}}},
    {{{4007, 0, 0}, 0, {-1526, 16}, {203, 98, 97, 255}}},
    {{{4007, 0, 4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{4783, 524, 4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{4783, 524, -2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{4783, 524, 4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{0, 524, 4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{-2, 524, 5360}, 0, {4004, 1992}, {202, 92, 91, 255}}},
    {{{5780, 524, 5360}, 0, {1470, 2024}, {202, 93, 92, 255}}},
    {{{5780, 524, 12}, 0, {-1523, 1983}, {203, 98, 97, 255}}},
    {{{4783, 524, -2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{4007, 0, 4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{0, 524, 4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{4783, 524, 4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{-4007, 0, -4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{-4783, 524, -4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{-4783, 524, 2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{-4783, 524, -4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{0, 524, -4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{2, 524, -5360}, 0, {4004, 1992}, {202, 92, 91, 255}}},
    {{{-5780, 524, -5360}, 0, {1470, 2024}, {202, 93, 92, 255}}},
    {{{-5780, 524, -12}, 0, {-1523, 1983}, {203, 98, 97, 255}}},
    {{{-4783, 524, 2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{-4007, 0, -4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{0, 0, -4007}, 0, {3999, -32}, {202, 92, 91, 255}}},
    {{{0, 524, -4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{-4783, 524, -4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{4783, 524, 2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{4783, 524, -4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{4007, 0, -4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{4783, 524, -4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{4783, 524, 2}, 0, {-1529, 1085}, {203, 98, 97, 255}}},
    {{{5780, 524, -12}, 0, {-1523, 1983}, {203, 98, 97, 255}}},
    {{{5780, 524, -5360}, 0, {1470, 2024}, {202, 93, 92, 255}}},
    {{{-2, 524, -5360}, 0, {4004, 1992}, {202, 92, 91, 255}}},
    {{{0, 524, -4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
    {{{4007, 0, -4007}, 0, {716, 11}, {202, 93, 92, 255}}},
    {{{4783, 524, -4662}, 0, {1118, 1111}, {202, 93, 92, 255}}},
    {{{0, 524, -4662}, 0, {4001, 1073}, {202, 92, 91, 255}}},
};


Vtx Cluster_Grid_color[] = {
    {{{4007, 0, -4007}, 0, {-3613, 5661}, {202, 93, 92, 255}}},
    {{{-4007, 0, -4007}, 0, {5661, 5661}, {202, 93, 92, 255}}},
    {{{-4007, 0, 4007}, 0, {5661, -3613}, {202, 93, 92, 255}}},
    {{{4007, 0, 4007}, 0, {-3613, -3613}, {202, 93, 92, 255}}},
};


Gfx Cluster_model_gfx[] = {
    // Material Rock
    gsSPDisplayList(Mars_Rock),
    // End Material Rock
    gsSPVertex(&Cluster_Grid_001_color[0], 30, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(4, 5, 6, 0, 7, 4, 6, 0),
    gsSP2Triangles(4, 7, 8, 0, 9, 4, 8, 0),
    gsSP2Triangles(10, 11, 12, 0, 10, 12, 13, 0),
    gsSP2Triangles(14, 15, 16, 0, 14, 16, 17, 0),
    gsSP2Triangles(18, 19, 20, 0, 21, 18, 20, 0),
    gsSP2Triangles(18, 21, 22, 0, 23, 18, 22, 0),
    gsSP2Triangles(24, 13, 25, 0, 24, 25, 26, 0),
    gsSP2Triangles(0, 27, 28, 0, 0, 28, 29, 0),
    gsSPVertex(&Cluster_Grid_001_color[14], 1, 0),
    gsSPVertex(&Cluster_Grid_001_color[30], 22, 1),
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
    gsSPVertex(&Cluster_Grid_color[0], 4, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    // Begin decor
    gsSPEndDisplayList(),
};
struct BaseDefinition Cluster_Bases[] = {
    {.position = {0, 3300}, .startingTeam = TEAM(0)},
    {.position = {1000, 600}, .startingTeam = TEAM(1)},
    {.position = {-1000, 600}, .startingTeam = TEAM(2)},
    {.position = {-1000, -1400}, .startingTeam = TEAM(1)},
    {.position = {1000, -1400}, .startingTeam = TEAM(2)},
    {.position = {1000, -3400}, .startingTeam = TEAM(1)},
    {.position = {-1000, -3400}, .startingTeam = TEAM(2)},
};

struct Vector2 Cluster_StartingPositions[] = {
    {0, 3300},
    {1000, -3400},
    {-1000, -3400},
};

struct SceneBoundary Cluster_Boundary[] = {
    {{4046.9, -4046.9}, {-1, 0}, 8093.81},
    {{4046.9, 4046.9}, {-0, -1}, 8093.81},
    {{-4046.9, 4046.9}, {1, 0}, 8093.81},
    {{-4046.9, -4046.9}, {-0, 1}, 8093.81},
};

struct DecorDefinition Cluster_Decor[] = {
};
struct Vector3 Cluster_PathingNodes[] = {
    {1007.66, 0.0f, -1402.5},
    {-996.644, 0.0f, -1382.1},
    {-995.229, 0.0f, -3377.29},
    {15.9748, 0.0f, 3307.41},
    {999.1, 0.0f, 593.985},
    {-996.644, 0.0f, 603.816},
    {981.638, 0.0f, -3411.15},
};
char Cluster_BasesPathNodes[] = {
    3,
    4,
    5,
    1,
    0,
    6,
    2,
};
unsigned short Cluster_NodeDistances[] = {
    0, 2004, 2812, 4813, 1996, 2835, 2008, 
    2004, 0, 1995, 4797, 2808, 1985, 2833, 
    2812, 1995, 0, 6760, 4443, 3981, 1977, 
    4813, 4797, 6760, 0, 2886, 2887, 6787, 
    1996, 2808, 4443, 2886, 0, 1995, 4005, 
    2835, 1985, 3981, 2887, 1995, 0, 4475, 
    2008, 2833, 1977, 6787, 4005, 4475, 0, 
};
char Cluster_NextNode[] = {
    0, 1, 2, 3, 4, 5, 6, 
    0, 1, 2, 3, 4, 5, 6, 
    0, 1, 2, 3, 4, 1, 6, 
    0, 1, 2, 3, 4, 5, 6, 
    0, 1, 2, 3, 4, 5, 0, 
    0, 1, 1, 3, 4, 5, 6, 
    0, 1, 2, 3, 0, 5, 6, 
};
struct LevelDefinition Cluster_Definition = {
    .maxPlayerCount = 3,
    .playerStartLocations = Cluster_StartingPositions,
    .baseCount = 7,
    .decorCount = 0,
    .bases = Cluster_Bases,
    .decor = Cluster_Decor,
    .levelBoundaries = {{-4046.9, -4046.9}, {4046.9, 4046.9}},
    .sceneRender = Cluster_model_gfx,
    .theme = &MarsTheme,
    .staticScene = {Cluster_Boundary, 4},
    .pathfinding = {.nodeCount = 7, .baseNodes = Cluster_BasesPathNodes, .nodeDistances = Cluster_NodeDistances, .nodePositions = Cluster_PathingNodes, .nextNode = Cluster_NextNode},
};

