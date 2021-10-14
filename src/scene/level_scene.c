
#include "level_scene.h"

#define DEFAULT_NEAR       

void levelSceneInit(struct LevelScene* levelScene, unsigned int playercount) {
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        cameraInit(&levelScene->cameras[i], 33.0f, 400.0f, 2000.0f);
    }
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        cameraSetupMatrices(&levelScene->cameras[i], renderState, 320.0f / 240.0f);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);
    }
}
