#ifndef _LEVEL_SCENE_H
#define _LEVEL_SCENE_H

#include <ultra64.h>

#include "game_defs.h"
#include "camera.h"
#include "minion.h"
#include "graphics/render_state.h"

#define MAX_MINIONS_PER_BASE       3

struct LevelScene {
    struct Camera cameras[MAX_PLAYERS];
    unsigned int playerCount;

    unsigned int baseCount;
    
    Gfx* levelDL;
    struct Minion* minions;
    unsigned int minionCount;
};

void levelSceneInit(struct LevelScene* levelScene, unsigned int playercount, unsigned int baseCount);
void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState);
void levelSceneUpdate(struct LevelScene* levelScene);

void levelSceneSpawnMinion(struct LevelScene* levelScene, enum MinionType type, struct Transform* at);

#endif