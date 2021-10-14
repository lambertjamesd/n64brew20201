
#include "level_scene.h"

void initLevelScene(struct LevelScene* levelScene, unsigned int playercount) {
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        transformInitIdentity(&levelScene->cameras[i].transform);
    }
}