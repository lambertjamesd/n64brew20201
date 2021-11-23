#include "level.h"
#include "../../level_themes/Space/theme.h"
#include "sk64/skelatool_defs.h"
#include <ultra64.h>

Gfx Spaceship_Template_model_gfx[] = {
    // Begin decor
    gsSPDisplayList(Space_Lockers),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 4, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_LockersDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPDisplayList(Space_SSCrate),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 2, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_CrateDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPDisplayList(Space_SetTextures2),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 0, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPDisplayList(Space_ChairDisplayList),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 1, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
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
struct BaseDefinition Spaceship_Template_Bases[] = {
};

struct Vector2 Spaceship_Template_StartingPositions[] = {
};

struct SceneBoundary Spaceship_Template_Boundary[] = {
    {{6725.82, 2792.49}, {-0.666409, -0.745586}, 5924.63},
    {{2308.5, 6740.71}, {-0.48001, -0.877263}, 1315.73},
    {{1154.26, 7372.28}, {-0.122669, -0.992448}, 1163.03},
    {{0.0129539, 7514.95}, {0.122669, -0.992448}, 1163.03},
    {{-1154.23, 7372.28}, {0.48001, -0.877263}, 1315.73},
    {{-2308.47, 6740.71}, {0.400455, 0.916317}, 9859.35},
};

struct DecorDefinition Spaceship_Template_Decor[] = {
    {{-1372.11, 0, -486.491}, {0, 0, 0, 1}, 1, CHAIR_DECOR_ID},
    {{-1791.03, 0, -62.0912}, {0, 0, 0, 1}, 1, CONSOLE_DECOR_ID},
    {{-1302.27, 0, -191.91}, {0, 0, 0, 1}, 1, CRATE_DECOR_ID},
    {{-1732.28, 0, -511.29}, {0, 0, 0, 1}, 1, FOOD_DISHES_DECOR_ID},
    {{-652.158, 0, -538.1}, {0, 0, 0, 1}, 1, LOCKERS_DECOR_ID},
    {{-2440.91, 0, -170.898}, {0, 0, 0, 1}, 1, TABLE_DECOR_ID},
};
struct Vector3 Spaceship_Template_PathingNodes[] = {
};
char Spaceship_Template_BasesPathNodes[] = {
};
unsigned short Spaceship_Template_NodeDistances[] = {
};
char Spaceship_Template_NextNode[] = {
};
struct LevelDefinition Spaceship_Template_Definition = {
    .maxPlayerCount = 0,
    .playerStartLocations = Spaceship_Template_StartingPositions,
    .baseCount = 0,
    .decorCount = 6,
    .bases = Spaceship_Template_Bases,
    .decor = Spaceship_Template_Decor,
    .levelBoundaries = {{-2308.47, 0}, {6725.82, 7514.95}},
    .sceneRender = Spaceship_Template_model_gfx,
    .theme = &SpaceTheme,
    .staticScene = {Spaceship_Template_Boundary, 6},
    .pathfinding = {.nodeCount = 0, .baseNodes = Spaceship_Template_BasesPathNodes, .nodeDistances = Spaceship_Template_NodeDistances, .nodePositions = Spaceship_Template_PathingNodes, .nextNode = Spaceship_Template_NextNode},
};

