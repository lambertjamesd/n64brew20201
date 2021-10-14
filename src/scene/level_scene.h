#ifndef _LEVEL_SCENE_H
#define _LEVEL_SCENE_H

#include <ultra64.h>

#include "game_defs.h"
#include "camera.h"

struct LevelScene {
    struct Camera cameras[MAX_PLAYERS];
    unsigned int playerCount;
    
    Gfx* levelDL;
};

void initLevelScene(struct LevelScene* levelScene, unsigned int playercount);

#endif