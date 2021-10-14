#ifndef _LEVEL_SCENE_H
#define _LEVEL_SCENE_H

#include <ultra64.h>

#include "game_defs.h"
#include "camera.h"
#include "graphics/render_state.h"

struct LevelScene {
    struct Camera cameras[MAX_PLAYERS];
    unsigned int playerCount;
    
    Gfx* levelDL;
};

void levelSceneInit(struct LevelScene* levelScene, unsigned int playercount);
void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState);

#endif