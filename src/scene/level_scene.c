
#include "level_scene.h"
#include "util/memory.h"
#include "controls/controller.h"
#include "util/time.h"
#include "collision/dynamicscene.h"

#define DEFAULT_NEAR       

void levelSceneInit(struct LevelScene* levelScene, struct LevelDefinition* definition, unsigned int playercount) {
    dynamicSceneInit();

    levelScene->levelDL = definition->sceneRender;
    
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        cameraInit(&levelScene->cameras[i], 45.0f, 50.0f, 6000.0f);
        playerInit(&levelScene->players[i], i, &definition->playerStartLocations[i]);
    }

    levelScene->cameras[0].transform.position.z = 200.0f;
    levelScene->cameras[0].transform.position.y = 400.0f;
    quatAxisAngle(&gRight, -M_PI * 0.3333f, &levelScene->cameras[0].transform.rotation);

    levelScene->baseCount = definition->baseCount;
    levelScene->bases = malloc(sizeof(struct LevelBase) * definition->baseCount);
    for (unsigned i = 0; i < definition->baseCount; ++i) {
        levelBaseInit(&levelScene->bases[i], &definition->bases[i], (unsigned char)i, i >= playercount);
    }

    levelScene->minionCount = definition->baseCount * MAX_MINIONS_PER_BASE;
    levelScene->minions = malloc(sizeof(struct Minion) * levelScene->minionCount);
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        levelScene->minions[i].minionFlags = 0;
    }
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        cameraSetupMatrices(&levelScene->cameras[i], renderState, 320.0f / 240.0f);
        Mtx* scaleMatrix = renderStateRequestMatrices(renderState, 1);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);

        for (unsigned int i = 0; i < levelScene->baseCount; ++i) {
            levelBaseRender(&levelScene->bases[i], renderState);
        }

        for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
            playerRender(&levelScene->players[i], renderState);
        }

        for (unsigned int minionIndex = 0; minionIndex < levelScene->minionCount; ++minionIndex) {
            if (levelScene->minions[minionIndex].minionFlags & MinionFlagsActive) {
                minionRender(&levelScene->minions[minionIndex], renderState);
            }
        }
    }

}

void levelSceneUpdate(struct LevelScene* levelScene) {
    for (unsigned playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
        struct PlayerInput playerInput;
        playerInputPopulateWithJoystickData(
            controllersGetControllerData(playerIndex), 
            controllerGetLastButton(playerIndex), 
            &levelScene->cameras[playerIndex].transform.rotation,
            &playerInput
        );

        playerUpdate(&levelScene->players[playerIndex], &playerInput);
        struct Vector3 target = levelScene->players[playerIndex].transform.position;
        vector3AddScaled(&target, &gUp, 2.0f * SCENE_SCALE, &target);
        struct Vector3 velocityLeader;
        vector3Scale(&levelScene->players[playerIndex].velocity, &velocityLeader, SCENE_SCALE * 0.75f);
        velocityLeader.y = 0.0f;
        vector3Add(&target, &velocityLeader, &target);
        cameraUpdate(&levelScene->cameras[playerIndex], &target, 15.0f * SCENE_SCALE, 5.0f * SCENE_SCALE);
    }


    for (unsigned int minionIndex = 0; minionIndex < levelScene->minionCount; ++minionIndex) {
        if (levelScene->minions[minionIndex].minionFlags & MinionFlagsActive) {
            minionUpdate(&levelScene->minions[minionIndex]);
        }
    }

    for (unsigned int baseIndex = 0; baseIndex < levelScene->baseCount; ++baseIndex) {
        levelBaseUpdate(&levelScene->bases[baseIndex]);
    }

    dynamicSceneCollide();
}

void levelSceneSpawnMinion(struct LevelScene* levelScene, enum MinionType type, struct Transform* at, unsigned char baseId) {
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        if (!(levelScene->minions[i].minionFlags & MinionFlagsActive)) {
            minionInit(&levelScene->minions[i], type, at, baseId);
            break;
        }
    }
}