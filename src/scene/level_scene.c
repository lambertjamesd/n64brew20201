
#include "level_scene.h"

void levelSceneInit(struct LevelScene* levelScene, unsigned int playercount) {
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        transformInitIdentity(&levelScene->cameras[i].transform);
    }
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        cameraSetupMatrices(&levelScene->cameras[i], renderState, 320.0f / 240.0f);
    }
}
