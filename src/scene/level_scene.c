
#include "level_scene.h"
#include "util/memory.h"

#define DEFAULT_NEAR       

void levelSceneInit(struct LevelScene* levelScene, unsigned int playercount, unsigned int baseCount) {
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        cameraInit(&levelScene->cameras[i], 33.0f, 400.0f, 2000.0f);
    }

    levelScene->baseCount = baseCount;
    levelScene->minionCount = baseCount * MAX_MINIONS_PER_BASE;
    levelScene->minions = malloc(sizeof(struct Minion) * levelScene->minionCount);

    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        levelScene->minions[i].minionFlags = 0;
    }
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        cameraSetupMatrices(&levelScene->cameras[i], renderState, 320.0f / 240.0f);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);

        for (unsigned int minionIndex = 0; minionIndex < levelScene->minionCount; ++minionIndex) {
            if (levelScene->minions[minionIndex].minionFlags & MinionFlagsActive) {
                minionRender(&levelScene->minions[minionIndex], renderState);
            }
        }
    }
}

void levelSceneSpawnMinion(struct LevelScene* levelScene, enum MinionType type, struct Transform* at) {
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        if (!(levelScene->minions[i].minionFlags & MinionFlagsActive)) {
            minionInit(&levelScene->minions[i], type, at);
            break;
        }
    }
}