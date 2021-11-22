
#include <stdbool.h>
#include "level_scene.h"
#include "util/memory.h"
#include "controls/controller.h"
#include "util/time.h"
#include "collision/dynamicscene.h"
#include "assert.h"
#include "graphics/gfx.h"
#include "../data/gameplaymenu/menu.h"
#include "../data/models/characters.h"
#include "graphics/sprite.h"
#include "menu/basecommandmenu.h"
#include "menu/playerstatusmenu.h"
#include "minimap.h"
#include "audio/dynamic_music.h"
#include "events.h"
#include "collision/collisionlayers.h"
#include "levels/themedefinition.h"
#include "sk64/skelatool_defs.h"
#include "scene_management.h"
#include "tutorial/tutorial.h"
#include "menu/endgamemenu.h"
#include "../data/fonts/fonts.h"

#include "collision/polygon.h"
#include "math/vector3.h"

struct DynamicMarker gIntensityMarkers[] = {
    {0, {127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {25, {127, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {50, {127, 0, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {75, {127, 0, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
};

#define GO_SHOW_DURATION 0.5f
#define GAME_START_DELAY 5.0f
#define GAME_END_DELAY  5.0f
#define LOSE_BY_KNOCKOUT_TIME   15.0f

#define WIN_BY_PRESSING_START   1

void levelSceneInit(struct LevelScene* levelScene, struct LevelDefinition* definition, unsigned int playercount, unsigned char humanPlayerCount, enum LevelMetadataFlags flags) {
    levelScene->definition = definition;
    dynamicSceneInit(&gDynamicScene);
    endGameMenuResetStats();

    levelScene->levelDL = definition->sceneRender;
    levelScene->levelFlags = flags;
    levelScene->knockoutTimer = LOSE_BY_KNOCKOUT_TIME;
    levelScene->gameTimer = 0.0f;

    levelScene->baseCount = definition->baseCount;
    levelScene->bases = malloc(sizeof(struct LevelBase) * definition->baseCount);
    for (unsigned i = 0; i < definition->baseCount; ++i) {
        levelBaseInit(&levelScene->bases[i], &definition->bases[i], (unsigned char)i, definition->bases[i].startingTeam >= playercount);
    }
    
    levelScene->playerCount = playercount;

    //initializing player controlled characters 
    for(unsigned i = 0; i < playercount; ++i){
        playerInit(&levelScene->players[i], i, i, &definition->playerStartLocations[i]);
        controlsScramblerInit(&levelScene->scramblers[i]);
        cameraInit(&levelScene->cameras[i], 45.0f, 100.0f, 18000.0f);
        vector3AddScaled(&levelScene->players[i].transform.position, &gForward, SCENE_SCALE * 2.0f, &levelScene->cameras[i].transform.position);
        vector3AddScaled(&levelScene->cameras[i].transform.position, &gUp, SCENE_SCALE * 2.0f, &levelScene->cameras[i].transform.position);
        baseCommandMenuInit(&levelScene->baseCommandMenu[i]);
        gPlayerAtBase[i] = 0;
        
    }
    //initializing AI controlled characters 
    unsigned numBots = playercount - humanPlayerCount;
    levelScene->botsCount = numBots;
    if(numBots > 0){
        levelScene->bots = malloc(sizeof(struct AIController) * numBots);
        for (unsigned i = humanPlayerCount; i < playercount; ++i) {
            ai_Init(&levelScene->bots[i - humanPlayerCount], &definition->pathfinding, i, i, levelScene->baseCount);
        }
    }
    
    levelScene->decorMatrices = malloc(sizeof(Mtx) * definition->decorCount);

    for (unsigned i = 0; i < definition->decorCount; ++i) {
        struct Transform decorTransform;
        decorTransform.position = definition->decor[i].position;
        decorTransform.rotation = definition->decor[i].rotation;
        vector3Scale(&gOneVec, &decorTransform.scale, definition->decor[i].scale);
        transformToMatrixL(&decorTransform, &levelScene->decorMatrices[i]);

        unsigned id = definition->decor[i].decorID;

        if (definition->theme->decorShapes[id]) {
            struct Vector2 pos2D;
            pos2D.x = decorTransform.position.x;
            pos2D.y = decorTransform.position.z;
            struct DynamicSceneEntry* entry = dynamicSceneNewEntry(definition->theme->decorShapes[id], 0, &pos2D, 0, 0, CollisionLayersTangible | CollisionLayersStatic);
            dynamicEntrySetRotation3D(entry, &decorTransform.rotation);
            dynamicEntrySetScale(entry, decorTransform.scale.x);
        }
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

    itemDropsInit(&levelScene->itemDrops);

    levelScene->humanPlayerCount = humanPlayerCount;

    gfxInitSplitscreenViewport(humanPlayerCount);

    levelScene->state = LevelSceneStateIntro;
    textBoxInit(&gTextBox, "Ready?", 200, SCREEN_WD / 2, SCREEN_HT / 2);
    levelScene->stateTimer = GAME_START_DELAY;
    levelScene->winningTeam = TEAM_NONE;

    // dynamicMusicUseMarkers(gIntensityMarkers, sizeof(gIntensityMarkers) / sizeof(*gIntensityMarkers));

    osWritebackDCache(&gSplitScreenViewports[0], sizeof(gSplitScreenViewports));

    if (flags & LevelMetadataFlagsTutorial) {
        tutorialInit(levelScene);
    }
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gUseSolidColor);
    spriteSetLayer(renderState, LAYER_MENU_BORDER, gUseMenuBorder);
    spriteSetLayer(renderState, LAYER_BUTTONS, gUseButtonsIcon);
    spriteSetLayer(renderState, LAYER_COMMAND_BUTTONS, gUseCommandsTexture);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);
    spriteSetLayer(renderState, LAYER_UPGRADE_ICONS, gUseUpgradeIcons);

    // render minions
    Gfx* minionGfx = renderStateAllocateDLChunk(renderState, MINION_GFX_PER_MINION * levelScene->minionCount + 2);
    Gfx* prevDL = renderStateReplaceDL(renderState, minionGfx);
    gSPDisplayList(renderState->dl++, mat_Minion_f3d_material);
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
    Gfx* playerGfx = renderStateAllocateDLChunk(renderState, PLAYER_GFX_PER_PLAYER * levelScene->playerCount + 3);
    prevDL = renderStateReplaceDL(renderState, playerGfx);
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        playerRender(&levelScene->players[i], renderState);
    }
    gDPPipeSync(renderState->dl++);
	gDPSetTextureLUT(renderState->dl++, G_TT_NONE);
    gSPEndDisplayList(renderState->dl++);
    Gfx* playerEnd = renderStateReplaceDL(renderState, prevDL);
    assert(playerEnd <= playerGfx + PLAYER_GFX_PER_PLAYER * levelScene->playerCount + 1);

    Gfx* itemDropsGfx = itemDropsRender(&levelScene->itemDrops, renderState);

    if (levelScene->levelFlags & LevelMetadataFlagsTutorial) {
        tutorialRender(levelScene, renderState);
    }

    gSPEndDisplayList(renderState->transparentDL++);

    for (unsigned int i = 0; i < levelScene->humanPlayerCount; ++i) {
        gDPPipeSync(renderState->dl++);
        Vp* viewport = &gSplitScreenViewports[i];
        cameraSetupMatrices(
            &levelScene->cameras[i], 
            renderState, 
            (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1],
            controlsScramblerGetCameraRotation(&levelScene->scramblers[i])
        );
        renderState->cameraRotation = &levelScene->cameras[i].transform.rotation;
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
        gSPSegment(renderState->dl++, MATRIX_TRANSFORM_SEGMENT, levelScene->decorMatrices);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);
        gSPDisplayList(renderState->dl++, baseGfx);
        gSPDisplayList(renderState->dl++, playerGfx);
        gSPDisplayList(renderState->dl++, minionGfx);
        gSPDisplayList(renderState->dl++, itemDropsGfx);
        gSPDisplayList(renderState->dl++, renderState->transparentQueueStart);

        gSPDisplayList(renderState->dl++, mat_Dizzy_Dizzy);
        for (unsigned playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
            controlsScramblerRender(&levelScene->scramblers[playerIndex], &levelScene->players[playerIndex], renderState);
        }
        gSPDisplayList(renderState->dl++, mat_revert_Dizzy_Dizzy);

        baseCommandMenuRender(
            &levelScene->baseCommandMenu[i], 
            renderState, 
            &gClippingRegions[i * 4]
        );
        playerStatusMenuRender(&levelScene->players[i], renderState, levelScene->winningTeam, levelScene->knockoutTimer < LOSE_BY_KNOCKOUT_TIME ? levelScene->knockoutTimer : -1.0f, &gClippingRegions[i * 4]);
    }

    gSPViewport(renderState->dl++, osVirtualToPhysical(&gFullScreenVP));
    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );

    minimapRender(levelScene, renderState, gViewportPosition[levelScene->humanPlayerCount-1].minimapLocation);

    textBoxRender(&gTextBox, renderState);

    spriteFinish(renderState);
}

void leveSceneUpdateCamera(struct LevelScene* levelScene, unsigned playerIndex) {
    struct Vector3 target = levelScene->players[playerIndex].transform.position;
    vector3AddScaled(&target, &gUp, 2.0f * SCENE_SCALE, &target);

    if (levelScene->cameras[playerIndex].mode == CameraModeFollow) {
        struct Vector3 velocityLeader;
        vector3Scale(&levelScene->players[playerIndex].velocity, &velocityLeader, SCENE_SCALE * 0.5f);
        velocityLeader.y = 0.0f;
        vector3Add(&target, &velocityLeader, &target);
    }

    if (controllerGetButtonDown(playerIndex, Z_TRIG)) {
        cameraSetLockedMode(&levelScene->cameras[playerIndex], &levelScene->players[playerIndex].transform.rotation);
    } else if (controllerGetButtonUp(playerIndex, Z_TRIG)) {
        cameraSetFollowMode(&levelScene->cameras[playerIndex]);
    }

    cameraUpdate(&levelScene->cameras[playerIndex], &target, 15.0f * SCENE_SCALE, 5.0f * SCENE_SCALE);
}

unsigned short levelSceneCaluclateIntensity() {
    if (gTimePassed - gLastDeathTime < 5.0f) {
        return 75;
    } else if (gTimePassed - gLastDamageTime < 5.0f) {
        return 50;
    } else if (gTimePassed - gLastCaptureTime < 5.0f) {
        return 25;
    } else {
        return 0;
    }
}

void levelSceneUpdateMusic(struct LevelScene* levelScene) {
    unsigned short targetIntensity = levelSceneCaluclateIntensity();
    unsigned short currentIntesnity = dynamicMusicGetIntensity();

    if (targetIntensity < currentIntesnity) {
        dynamicMusicSetIntensity(currentIntesnity - 1);
    } else if (targetIntensity > currentIntesnity) {
        dynamicMusicSetIntensity(currentIntesnity + 1);
    }
}

void levelSceneCollectHumanPlayerInput(struct LevelScene* levelScene, unsigned playerIndex, struct PlayerInput* playerInput) {
    if (baseCommandMenuIsShowing(&levelScene->baseCommandMenu[playerIndex])) {
        playerInputNoInput(playerInput);
    } else {
        struct Quaternion cameraRotation;
        float cameraRoll = controlsScramblerGetCameraRotation(&levelScene->scramblers[playerIndex]);

        if (cameraRoll) {
            struct Quaternion roll;
            quatAxisAngle(&gForward, cameraRoll, &roll);
            quatMultiply(&levelScene->cameras[playerIndex].transform.rotation, &roll, &cameraRotation);
        } else {
            cameraRotation = levelScene->cameras[playerIndex].transform.rotation;
        }

        playerInputPopulateWithJoystickData(
            controllersGetControllerData(playerIndex), 
            &cameraRotation,
            controlsScramblerIsActive(&levelScene->scramblers[playerIndex], ControlsScramblerDPADSwap) ? PlayerInputFlagsSwapJoystickAndDPad : 0,
            playerInput
        );
    }
}

void levelSceneCollectPlayerInput(struct LevelScene* levelScene, unsigned playerIndex, struct PlayerInput* playerInput) {
    if (levelScene->state == LevelSceneStatePlaying) {
        if (playerIndex < levelScene->humanPlayerCount) {
            levelSceneCollectHumanPlayerInput(levelScene, playerIndex, playerInput);
        } else if (!(levelScene->levelFlags & LevelMetadataFlagsTutorial)) {
            ai_collectPlayerInput(levelScene, &levelScene->bots[playerIndex - levelScene->humanPlayerCount], playerInput);
        } else {
            playerInputNoInput(playerInput);
        }
    } else {
        playerInputNoInput(playerInput);
        baseCommandMenuHide(&levelScene->baseCommandMenu[playerIndex]);
    }
}

float gNextSampletime = 0.0f;

void levelSceneCollectStats(struct LevelScene* levelScene) {
    unsigned baseCount[MAX_PLAYERS];

    for (unsigned i = 0; i < levelScene->playerCount; ++i) {
        baseCount[i] = 0;
    }

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        unsigned baseFaction = levelBaseGetTeam(&levelScene->bases[i]);
        ++baseCount[baseFaction];
    }

    unsigned baseHaverCount = 0;

    for (unsigned i = 0; i < levelScene->playerCount; ++i) {
        levelScene->players[i].controlledBases = baseCount[i];

        statTrackerLogDatapoint(&gPlayerBaseStats[i], (float)baseCount[i]);

        if (baseCount[i]) {
            ++baseHaverCount;
        }
    }

    if (baseHaverCount > 1 || (levelScene->levelFlags & LevelMetadataFlagsTutorial)) {
        levelScene->knockoutTimer = LOSE_BY_KNOCKOUT_TIME;
    } else {
        levelScene->knockoutTimer -= gTimeDelta;
    }
}

void levelSceneUpdate(struct LevelScene* levelScene) {
    levelSceneCollectStats(levelScene);
    levelScene->winningTeam = levelSceneFindWinningTeam(levelScene);

    if (levelScene->winningTeam != TEAM_NONE && levelScene->state != LevelSceneStateDone) {
        levelScene->state = LevelSceneStateDone;
        levelScene->stateTimer = GAME_END_DELAY;
    }

    if (levelScene->state == LevelSceneStateDone) {
        levelScene->stateTimer -= gTimeDelta;

        if (levelScene->stateTimer < 0.0f) {
            sceneQueuePostGameScreen(levelScene->winningTeam, levelScene->playerCount, levelScene->gameTimer - GAME_END_DELAY - GAME_START_DELAY);
        }
    }

    levelScene->gameTimer += gTimeDelta;

    if (!(levelScene->levelFlags & LevelMetadataFlagsTutorial)) {
        for (unsigned i = 0; i < levelScene->botsCount; ++i) {
            ai_update(levelScene, &levelScene->bots[i]);
        }
    }

    for (unsigned playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
        struct PlayerInput* playerInput = &levelScene->scramblers[playerIndex].playerInput;

        controlsScramblerUpdate(&levelScene->scramblers[playerIndex]);

        levelSceneCollectPlayerInput(levelScene, playerIndex, playerInput);

        if (!playerIsAlive(&levelScene->players[playerIndex])) {
            baseCommandMenuHide(&levelScene->baseCommandMenu[playerIndex]);
        }

        controlsScramblerApply(&levelScene->scramblers[playerIndex]);

        baseCommandMenuUpdate(&levelScene->baseCommandMenu[playerIndex], playerIndex);
        playerUpdate(&levelScene->players[playerIndex], playerInput);
        leveSceneUpdateCamera(levelScene, playerIndex);

        if (levelScene->state == LevelSceneStatePlaying && levelScene->levelFlags & LevelMetadataFlagsTutorial) {
            tutorialUpdate(levelScene, playerInput);
        }
    }

    if (levelScene->state == LevelSceneStateIntro) {
        levelScene->stateTimer -= gTimeDelta;

        if (levelScene->stateTimer < 0.0f) {
            levelScene->state = LevelSceneStatePlaying;
            textBoxHide(&gTextBox);
        } else if (levelScene->stateTimer < GO_SHOW_DURATION) {
            if (!textBoxIsVisible(&gTextBox)) {
                textBoxInit(&gTextBox, "GO!", 200, SCREEN_WD / 2, SCREEN_HT / 2);
                gTextBox.currState = TextBoxStateShowing;
                gTextBox.animateTimer = 0.0f;
            }
        } else if (levelScene->stateTimer < GAME_START_DELAY * 0.5f) {
            textBoxHide(&gTextBox);
        }

        textBoxUpdate(&gTextBox);
        return;
    }

    for (unsigned int baseIndex = 0; baseIndex < levelScene->baseCount; ++baseIndex) {
        levelBaseUpdate(&levelScene->bases[baseIndex]);
    }

    for (unsigned finderIndex = 0; finderIndex < TARGET_FINDER_COUNT; ++finderIndex) {
        targetFinderUpdate(&levelScene->targetFinders[finderIndex]);
    }

    for (unsigned int minionIndex = 0; minionIndex < levelScene->minionCount; ++minionIndex) {
        if (levelScene->minions[minionIndex].minionFlags & MinionFlagsActive) {
            minionUpdate(&levelScene->minions[minionIndex]);
        }
    }

    itemDropsUpdate(&levelScene->itemDrops);

    dynamicSceneCollide();
    levelSceneUpdateMusic(levelScene);
    textBoxUpdate(&gTextBox);
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
            minionApplyDamage(&levelScene->minions[i], 100.0f);
        }
    }
}

void levelSceneIssueMinionCommand(struct LevelScene* levelScene, unsigned followingPlayer, enum MinionCommand command) {
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        struct Minion* minion = &levelScene->minions[i];
        if (minion->followingPlayer == followingPlayer && minionIsAlive(minion) && minion->currentCommand == MinionCommandFollow) {
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

int levelSceneFindWinningTeam(struct LevelScene* levelScene) {
    if (levelScene->state == LevelSceneStateDone) {
        return levelScene->winningTeam;
    }

#if WIN_BY_PRESSING_START
    for (unsigned i = 0; i < levelScene->playerCount; ++i) {
        if (controllerGetButtonDown(i, START_BUTTON)) {
            return i;
        }
    }
#endif

    int result = -1;

    for (unsigned i = 0; i < levelScene->baseCount; ++i) {
        if (levelScene->bases[i].state != LevelBaseStateNeutral &&
            levelScene->bases[i].team.teamNumber != result) {
            if (result == -1) {
                result = levelScene->bases[i].team.teamNumber;
            } else {
                return TEAM_NONE;
            }
        }
    }

    // winning by denying oponents control of bases for too long
    if (levelScene->knockoutTimer <= 0.0f && result != -1) {
        return result;
    }

    for (unsigned i = 0; i < levelScene->playerCount; ++i) {
        if (playerIsAlive(&levelScene->players[i]) &&
            levelScene->players[i].team.teamNumber != result) {
            if (result == -1) {
                result = levelScene->players[i].team.teamNumber;
            } else {
                return TEAM_NONE;
            }
        }
    }

    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        if (minionIsAlive(&levelScene->minions[i]) &&
            levelScene->minions[i].team.teamNumber != result) {
            if (result == -1) {
                result = levelScene->minions[i].team.teamNumber;
            } else {
                return TEAM_NONE;
            }
        }
    }

    return result;
}

void levelSceneApplyScrambler(struct LevelScene* levelScene, unsigned fromTeam, enum ControlsScramblerType scambler) {
    for (unsigned i = 0; i < levelScene->playerCount; ++i) {
        if (levelScene->players[i].team.teamNumber != fromTeam) {
            controlsScramblerTrigger(&levelScene->scramblers[i], scambler);
        }
    }
}

struct Player* levelGetClosestEnemyPlayer(struct LevelScene* forScene, struct Vector3* closeTo, unsigned team, float* outDist){
    struct Vector3* currPos = &forScene->players[0].transform.position;
    float minDist = vector3DistSqrd(currPos, closeTo);
    unsigned int minIndex = 0;

        for(unsigned entInd = 1; entInd < forScene->playerCount; ++entInd){
        if(forScene->players[entInd].team.teamNumber != team){
            currPos = &forScene->players[entInd].transform.position;
            float thisDist = vector3DistSqrd(currPos, closeTo);
            if(thisDist < minDist){
                minIndex = entInd;
                minDist = thisDist;
            }
        }
    }

    *outDist = minDist;
    return &forScene->players[minIndex];
}

struct Minion* levelGetClosestEnemyMinion(struct LevelScene* forScene, struct Vector3* closeTo, unsigned team, float* outDist){
    float minDist = 100000000.0f;
    unsigned int minIndex = ~0;

    for(unsigned entInd = 0; entInd < forScene->minionCount; ++entInd){
        if(minionIsAlive(&forScene->minions[entInd]) && forScene->minions[entInd].team.teamNumber != team){
            struct Vector3* currPos = &forScene->minions[entInd].transform.position;
            float thisDist = vector3DistSqrd(currPos, closeTo);
            if(thisDist < minDist){
                minIndex = entInd;
                minDist = thisDist;
            }
        }
    }

    if (minIndex == ~0) {
        return 0;
    }

    *outDist = minDist;
    return &forScene->minions[minIndex];
}