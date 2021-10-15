
#include "level_scene.h"
#include "util/memory.h"
#include "controls/controller.h"
#include "util/time.h"

#define DEFAULT_NEAR       

void levelSceneInit(struct LevelScene* levelScene, unsigned int playercount, unsigned int baseCount) {
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        cameraInit(&levelScene->cameras[i], 45.0f, 0.5f, 60.0f);
    }

    levelScene->cameras[0].transform.position.z = 2.0f;
    levelScene->cameras[0].transform.position.y = 4.0f;
    quatAxisAngle(&gRight, -M_PI * 0.3333f, &levelScene->cameras[0].transform.rotation);

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
        Mtx* scaleMatrix = renderStateRequestMatrices(renderState, 1);
        guScale(scaleMatrix, 1.0f / 256.0f, 1.0f / 256.0f, 1.0f / 256.0f);
        gSPMatrix(renderState->dl++, scaleMatrix, G_MTX_PUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);

        for (unsigned int minionIndex = 0; minionIndex < levelScene->minionCount; ++minionIndex) {
            if (levelScene->minions[minionIndex].minionFlags & MinionFlagsActive) {
                minionRender(&levelScene->minions[minionIndex], renderState);
            }
        }

        gSPPopMatrix(renderState->dl++, 1);
    }
}

void levelSceneUpdate(struct LevelScene* levelScene) {
    OSContPad* controllerData = controllersGetControllerData(0);

    struct Quaternion rotDelta;
    quatAxisAngle(&gUp, -controllerData->stick_x * gTimeDelta / 200.0f, &rotDelta);
    struct Quaternion tmpRot;
    quatMultiply(&rotDelta, &levelScene->cameras[0].transform.rotation, &tmpRot);
    quatAxisAngle(&gRight, controllerData->stick_y * gTimeDelta / 200.0f, &rotDelta);
    quatMultiply(&tmpRot, &rotDelta, &levelScene->cameras[0].transform.rotation);
    
    if (controllerData->button & (A_BUTTON | B_BUTTON)) {
        struct Vector3 forward;
        quatMultVector(&levelScene->cameras[0].transform.rotation, &gForward, &forward);
        vector3AddScaled(&levelScene->cameras[0].transform.position, &forward, (controllerData->button & A_BUTTON) ? -gTimeDelta : gTimeDelta, &levelScene->cameras[0].transform.position);
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