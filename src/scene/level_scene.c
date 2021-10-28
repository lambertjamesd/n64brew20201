
#include "level_scene.h"
#include "util/memory.h"
#include "controls/controller.h"
#include "util/time.h"
#include "collision/dynamicscene.h"
#include "assert.h"
#include "graphics/gfx.h"
#include "../data/menu/menu.h"
#include "graphics/sprite.h"
#include "menu/basecommandmenu.h"
#include "menu/gbfont.h"

static Vp gSplitScreenViewports[4];
static unsigned short gClippingRegions[4 * 4];

unsigned short gViewportPosition[] = {
    // Single player
    0, 0, SCREEN_WD, SCREEN_HT,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    // Two player
    0, 0, SCREEN_WD/2-1, SCREEN_HT,
    SCREEN_WD/2+1, 0, SCREEN_WD, SCREEN_HT,
    0, 0, 0, 0,
    0, 0, 0, 0,
    // Three player
    0, 0, SCREEN_WD/2-1, SCREEN_HT/2-1,
    SCREEN_WD/2+1, 0, SCREEN_WD, SCREEN_HT/2-1,
    0, SCREEN_HT/2+1, SCREEN_WD/2-1, SCREEN_HT,
    0, 0, 0, 0,
    // Four player
    0, 0, SCREEN_WD/2-1, SCREEN_HT/2-1,
    SCREEN_WD/2+1, 0, SCREEN_WD, SCREEN_HT/2-1,
    0, SCREEN_HT/2+1, SCREEN_WD/2-1, SCREEN_HT,
    SCREEN_WD/2+1, SCREEN_HT/2+1, SCREEN_WD, SCREEN_HT,
};

void levelSceneInit(struct LevelScene* levelScene, struct LevelDefinition* definition, unsigned int playercount, unsigned char humanPlayerCount) {
    dynamicSceneInit(&gDynamicScene);
    initGBFont();

    levelScene->levelDL = definition->sceneRender;
    
    levelScene->playerCount = playercount;

    for (unsigned i = 0; i < playercount; ++i) {
        cameraInit(&levelScene->cameras[i], 45.0f, 100.0f, 18000.0f);
        playerInit(&levelScene->players[i], i, i, &definition->playerStartLocations[i]);
        vector3AddScaled(&levelScene->players[i].transform.position, &gForward, SCENE_SCALE * 2.0f, &levelScene->cameras[i].transform.position);
        vector3AddScaled(&levelScene->cameras[i].transform.position, &gUp, SCENE_SCALE * 2.0f, &levelScene->cameras[i].transform.position);
        baseCommandMenuInit(&levelScene->baseCommandMenu[i]);
    }

    quatAxisAngle(&gRight, -M_PI * 0.3333f, &levelScene->cameras[0].transform.rotation);

    levelScene->baseCount = definition->baseCount;
    levelScene->bases = malloc(sizeof(struct LevelBase) * definition->baseCount);
    for (unsigned i = 0; i < definition->baseCount; ++i) {
        levelBaseInit(&levelScene->bases[i], &definition->bases[i], (unsigned char)i, definition->bases[i].startingTeam >= playercount);
    }

    levelScene->minionCount = definition->baseCount * MAX_MINIONS_PER_BASE;
    levelScene->lastMinion = 0;
    levelScene->minions = malloc(sizeof(struct Minion) * levelScene->minionCount);
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        levelScene->minions[i].minionFlags = 0;
    }

    for (unsigned finderIndex = 0; finderIndex < TARGET_FINDER_COUNT; ++finderIndex) {
        targetFinderInit(&levelScene->targetFinders[finderIndex], (levelScene->minionCount / TARGET_FINDER_COUNT) * finderIndex);
    }

    levelScene->humanPlayerCount = humanPlayerCount;

    // 4 numbers per viewport, 4 viewports per slot
    unsigned viewportBase = (levelScene->humanPlayerCount - 1) * 4 * 4;
    
    for (unsigned i = 0; i < levelScene->humanPlayerCount; ++i) {
        unsigned l = gViewportPosition[viewportBase + 0];
        unsigned t = gViewportPosition[viewportBase + 1];
        unsigned r = gViewportPosition[viewportBase + 2];
        unsigned b = gViewportPosition[viewportBase + 3];

        gSplitScreenViewports[i].vp.vscale[0] = (r - l) * 4 / 2;
        gSplitScreenViewports[i].vp.vscale[1] = (b - t) * 4 / 2;
        gSplitScreenViewports[i].vp.vscale[2] = G_MAXZ/2;
        gSplitScreenViewports[i].vp.vscale[3] = 0;

        gSplitScreenViewports[i].vp.vtrans[0] = (r + l) * 4 / 2;
        gSplitScreenViewports[i].vp.vtrans[1] = (b + t) * 4 / 2;
        gSplitScreenViewports[i].vp.vtrans[2] = G_MAXZ/2;
        gSplitScreenViewports[i].vp.vtrans[3] = 0;

        gClippingRegions[i * 4 + 0] = l;
        gClippingRegions[i * 4 + 1] = t;
        gClippingRegions[i * 4 + 2] = r;
        gClippingRegions[i * 4 + 3] = b;

        viewportBase += 4;
    }

    osWritebackDCache(&gSplitScreenViewports[0], sizeof(gSplitScreenViewports));
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gUseSolidColor);
    spriteSetLayer(renderState, LAYER_COMMAND_BUTTONS, gUseCommandIcons);
    spriteSetLayer(renderState, LAYER_GB_FONT, gUseFontTexture);
    spriteSetLayer(renderState, LAYER_UPGRADE_ICONS, gUseUpgradeIcons);

    // render minions
    Gfx* minionGfx = renderStateAllocateDLChunk(renderState, MINION_GFX_PER_MINION * levelScene->minionCount + 1);
    Gfx* prevDL = renderStateReplaceDL(renderState, minionGfx);
    for (unsigned int minionIndex = 0; minionIndex < levelScene->minionCount; ++minionIndex) {
        if (levelScene->minions[minionIndex].minionFlags & MinionFlagsActive) {
            minionRender(&levelScene->minions[minionIndex], renderState);
        }
    }
    gSPEndDisplayList(renderState->dl++);
    Gfx* minionEnd = renderStateReplaceDL(renderState, prevDL);
    assert(minionEnd <= minionGfx + MINION_GFX_PER_MINION * levelScene->minionCount + 1);

    // render bases
    Gfx* baseGfx = renderStateAllocateDLChunk(renderState, BASE_GFX_PER_BASE * levelScene->baseCount + 1);
    prevDL = renderStateReplaceDL(renderState, baseGfx);
    for (unsigned int i = 0; i < levelScene->baseCount; ++i) {
        levelBaseRender(&levelScene->bases[i], renderState);
    }
    gSPEndDisplayList(renderState->dl++);
    Gfx* baseEnd = renderStateReplaceDL(renderState, prevDL);
    assert(baseEnd <= baseGfx + MINION_GFX_PER_MINION * levelScene->baseCount + 1);

    // render players
    Gfx* playerGfx = renderStateAllocateDLChunk(renderState, PLAYER_GFX_PER_PLAYER * levelScene->playerCount + 1);
    prevDL = renderStateReplaceDL(renderState, playerGfx);
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        playerRender(&levelScene->players[i], renderState);
    }
    gSPEndDisplayList(renderState->dl++);
    Gfx* playerEnd = renderStateReplaceDL(renderState, prevDL);
    assert(playerEnd <= playerGfx + PLAYER_GFX_PER_PLAYER * levelScene->playerCount + 1);

    gSPEndDisplayList(renderState->transparentDL++);    

    for (unsigned int i = 0; i < levelScene->humanPlayerCount; ++i) {
        Vp* viewport = &gSplitScreenViewports[i];
        cameraSetupMatrices(&levelScene->cameras[i], renderState, (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1]);
        gSPViewport(renderState->dl++, osVirtualToPhysical(viewport));
        gDPSetScissor(
            renderState->dl++, 
            G_SC_NON_INTERLACE, 
            gClippingRegions[i * 4 + 0],
            gClippingRegions[i * 4 + 1],
            gClippingRegions[i * 4 + 2],
            gClippingRegions[i * 4 + 3]
        );
        gDPPipeSync(renderState->dl++);
        gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);
        gSPDisplayList(renderState->dl++, baseGfx);
        gSPDisplayList(renderState->dl++, playerGfx);
        gSPDisplayList(renderState->dl++, minionGfx);
        gSPDisplayList(renderState->dl++, renderState->transparentQueueStart);

        baseCommandMenuRender(
            &levelScene->baseCommandMenu[i], 
            renderState, 
            gClippingRegions[i * 4 + 0],
            gClippingRegions[i * 4 + 1],
            gClippingRegions[i * 4 + 2],
            gClippingRegions[i * 4 + 3]
        );
    }

    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );

    spriteFinish(renderState);
}

void levelSceneUpdate(struct LevelScene* levelScene) {
    for (unsigned playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
        struct PlayerInput playerInput;

        if (baseCommandMenuIsShowing(&levelScene->baseCommandMenu[playerIndex])) {
            playerInputNoInput(&playerInput);
        } else {
            playerInputPopulateWithJoystickData(
                controllersGetControllerData(playerIndex), 
                controllerGetLastButton(playerIndex), 
                &levelScene->cameras[playerIndex].transform.rotation,
                &playerInput
            );
        }


        baseCommandMenuUpdate(&levelScene->baseCommandMenu[playerIndex], playerIndex);
        playerUpdate(&levelScene->players[playerIndex], &playerInput);
        struct Vector3 target = levelScene->players[playerIndex].transform.position;
        vector3AddScaled(&target, &gUp, 2.0f * SCENE_SCALE, &target);
        struct Vector3 velocityLeader;
        vector3Scale(&levelScene->players[playerIndex].velocity, &velocityLeader, SCENE_SCALE * 0.5f);
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

    for (unsigned finderIndex = 0; finderIndex < TARGET_FINDER_COUNT; ++finderIndex) {
        targetFinderUpdate(&levelScene->targetFinders[finderIndex]);
    }

    dynamicSceneCollide();
}

void levelSceneSpawnMinion(struct LevelScene* levelScene, enum MinionType type, struct Transform* at, unsigned char baseId, unsigned team, enum MinionCommand defualtCommand, unsigned followPlayer) {
    unsigned searchStart = levelScene->lastMinion;

    do {
        if (!(levelScene->minions[levelScene->lastMinion].minionFlags & MinionFlagsActive)) {
            minionInit(&levelScene->minions[levelScene->lastMinion], type, at, baseId, team, defualtCommand, followPlayer);
            break;
        }
        levelScene->lastMinion = (levelScene->lastMinion + 1) % levelScene->minionCount;
    } while (searchStart != levelScene->lastMinion);
    
}

void levelBaseDespawnMinions(struct LevelScene* levelScene, unsigned char baseId) {
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        if (levelScene->minions[i].sourceBaseId == baseId) {
            minionCleanup(&levelScene->minions[i]);
        }
    }
}

void levelSceneIssueMinionCommand(struct LevelScene* levelScene, unsigned followingPlayer, enum MinionCommand command) {
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        struct Minion* minion = &levelScene->minions[i];
        if (minion->followingPlayer == followingPlayer && (minion->minionFlags & MinionFlagsActive) != 0 && minion->currentCommand == MinionCommandFollow) {
            minionIssueCommand(minion, command, followingPlayer);
        }
    }
}

struct Vector3* levelSceneFindRespawnPoint(struct LevelScene* levelScene, struct Vector3* closeTo, unsigned team) {
    struct Vector3* result = 0;
    float score = 0.0;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        struct LevelBase* base = &levelScene->bases[i];

        if (base->team.teamNumber == team && base->state != LevelBaseStateNeutral) {
            float baseScore = vector3DistSqrd(closeTo, &base->position);

            if (!result || baseScore < score) {
                result = &base->position;
                score = baseScore;
            }
        }
    }

    return result;
}