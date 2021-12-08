
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
#include "events.h"
#include "collision/collisionlayers.h"
#include "levels/themedefinition.h"
#include "sk64/skelatool_defs.h"
#include "scene_management.h"
#include "tutorial/tutorial.h"
#include "menu/endgamemenu.h"
#include "team_data.h"
#include "../data/fonts/fonts.h"

#include "collision/polygon.h"
#include "math/vector3.h"

#define GO_SHOW_DURATION 0.5f
#define GAME_START_DELAY 3.0f
#define GAME_END_DELAY  5.0f
#define LOSE_BY_KNOCKOUT_TIME   15.0f

#define WIN_BY_PRESSING_L   1

struct LevelScene gCurrentLevel;

void levelSceneInit(struct LevelScene* levelScene, struct LevelDefinition* definition, unsigned int playercount, unsigned aiPlayerMask, enum LevelMetadataFlags flags, float aiDifficulty) {
    soundPlayerPlay(definition->song, 1.0f, SoundPlayerPriorityMusic, SoundPlayerFlagsIsMusic, 0);

    struct Quaternion noRotation;
    quatIdent(&noRotation);
    for (unsigned i = 0; i < playercount; ++i) {
        struct Vector3 pos3D;
        pos3D.x = definition->playerStartLocations[i].x;
        pos3D.y = 0.0f;
        pos3D.z = definition->playerStartLocations[i].y;
        soundPlayerUpdateListener(i, &pos3D, &noRotation);
    }


    levelScene->definition = definition;
    dynamicSceneInit(
        &gDynamicScene, 
        definition->baseCount * (MAX_MINIONS_PER_BASE + 1) + 
        playercount * 4 + 
        definition->decorCount + 
        TARGET_FINDER_COUNT + 
        MAX_ITEM_DROP
    );
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

    //initializing player characters 
    for(unsigned i = 0; i < playercount; ++i){
        struct Player* player = &levelScene->players[i];
        playerInit(player, i, i, &definition->playerStartLocations[i]);
        controlsScramblerInit(&levelScene->scramblers[i]);
        
        struct Camera* camera = &levelScene->cameras[i];
        cameraInit(camera, 45.0f, 100.0f, 18000.0f);
        struct Vector3 axisVector;
        quatMultVector(&player->transform.rotation, &gForward, &axisVector);
        vector3AddScaled(&player->transform.position, &axisVector, SCENE_SCALE * 2.0f, &camera->transform.position);
        quatMultVector(&player->transform.rotation, &gUp, &axisVector);
        vector3AddScaled(&camera->transform.position, &axisVector, SCENE_SCALE * 2.0f, &camera->transform.position);
        camera->transform.rotation = player->transform.rotation;

        baseCommandMenuInit(&levelScene->baseCommandMenu[i]);
        gPlayerAtBase[i] = 0;
        
    }

    //initializing AI controlled characters 
    unsigned numBots = 0;

    levelScene->aiPlayerMask = aiPlayerMask;

    for (unsigned i = 0; i < playercount; ++i) {
        if (IS_PLAYER_AI(levelScene, i)) {
            ++numBots;
        }
    }

    levelScene->botsCount = numBots;
    unsigned botIndex = 0;
    if(numBots > 0){
        levelScene->bots = malloc(sizeof(struct AIController) * numBots);
        for (unsigned i = 0; i < playercount; ++i) {
            if (IS_PLAYER_AI(levelScene, i)) {
                ai_Init(&levelScene->bots[botIndex], &definition->pathfinding, i, i, levelScene->baseCount, aiDifficulty);
                ++botIndex;
            }
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
    zeroMemory(levelScene->minions, sizeof(struct Minion) * levelScene->minionCount);
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        levelScene->minions[i].minionFlags = 0;
    }

    for (unsigned finderIndex = 0; finderIndex < TARGET_FINDER_COUNT; ++finderIndex) {
        targetFinderInit(&levelScene->targetFinders[finderIndex], (levelScene->minionCount / TARGET_FINDER_COUNT) * finderIndex);
    }

    itemDropsInit(&levelScene->itemDrops, sceneIsCampaign() && (flags & LevelMetadataFlagsTutorial2) != 0);

    levelScene->humanPlayerCount = playercount - numBots;
    levelScene->aiPlayerMask = aiPlayerMask;

    gfxInitSplitscreenViewport(levelScene->humanPlayerCount);
    soundPlayerSetListenerCount(levelScene->humanPlayerCount);

    levelScene->state = LevelSceneStateIntro;
    textBoxInit(&gTextBox, "Ready?", 200, SCREEN_WD / 2, SCREEN_HT / 2);
    levelScene->stateTimer = GAME_START_DELAY;
    levelScene->winningTeam = TEAM_NONE;

    osWritebackDCache(&gSplitScreenViewports[0], sizeof(gSplitScreenViewports));

    if (sceneIsCampaign() && flags & (LevelMetadataFlagsTutorial | LevelMetadataFlagsTutorial2)) {
        tutorialInit(levelScene, (flags & LevelMetadataFlagsTutorial) ? TutorialStateMove : TutorialStateUpgrade);
    }
}

void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gUseSolidColor);
    spriteSetLayer(renderState, LAYER_MENU_BORDER, gUseMenuBorder);
    spriteSetLayer(renderState, LAYER_BUTTONS, gUseButtonsIcon);
    spriteSetLayer(renderState, LAYER_COMMAND_BUTTONS, gUseCommandsTexture);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);
    spriteSetLayer(renderState, LAYER_HEALTH_BAR, gUseHealthBar);

    // render minions
    Gfx* minionGfx = renderStateAllocateDLChunk(renderState, MINION_GFX_PER_MINION * levelScene->minionCount + 3);
    Gfx* prevDL = renderStateReplaceDL(renderState, minionGfx);
    gSPDisplayList(renderState->dl++, gTeamTexture);
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
    Gfx* playerGfx = renderStateAllocateDLChunk(renderState, PLAYER_GFX_PER_PLAYER * levelScene->playerCount + 4);
    prevDL = renderStateReplaceDL(renderState, playerGfx);
    gSPDisplayList(renderState->dl++, gTeamTexture);
    for (unsigned int i = 0; i < levelScene->playerCount; ++i) {
        playerRender(&levelScene->players[i], renderState);
    }
    gDPPipeSync(renderState->dl++);
	gDPSetTextureLUT(renderState->dl++, G_TT_NONE);
    gSPEndDisplayList(renderState->dl++);
    Gfx* playerEnd = renderStateReplaceDL(renderState, prevDL);
    assert(playerEnd <= playerGfx + PLAYER_GFX_PER_PLAYER * levelScene->playerCount + 1);

    Gfx* itemDropsGfx = itemDropsRender(&levelScene->itemDrops, renderState);

    if (levelScene->levelFlags & (LevelMetadataFlagsTutorial | LevelMetadataFlagsTutorial2)) {
        tutorialRender(levelScene, renderState);
    }

    gSPEndDisplayList(renderState->transparentDL++);

    unsigned humanIndex = 0;

    for (unsigned int playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
        if (IS_PLAYER_AI(levelScene, playerIndex)) {
            continue;
        }

        gDPPipeSync(renderState->dl++);
        Vp* viewport = &gSplitScreenViewports[humanIndex];
        cameraSetupMatrices(
            &levelScene->cameras[playerIndex], 
            renderState, 
            (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1],
            controlsScramblerGetCameraRotation(&levelScene->scramblers[playerIndex])
        );
        renderState->cameraRotation = &levelScene->cameras[playerIndex].transform.rotation;
        gSPViewport(renderState->dl++, osVirtualToPhysical(viewport));

        unsigned short* clippingRegions = &gClippingRegions[humanIndex * 4];

        gDPSetScissor(
            renderState->dl++, 
            G_SC_NON_INTERLACE, 
            clippingRegions[0],
            clippingRegions[1],
            clippingRegions[2],
            clippingRegions[3]
        );
        gDPPipeSync(renderState->dl++);
        gSPClearGeometryMode(renderState->dl++, G_ZBUFFER | G_LIGHTING | G_CULL_BOTH);
        gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);

        if (levelScene->definition->theme->skybox) {
            Mtx* skyboxMatrix = renderStateRequestMatrices(renderState, 1);
            struct Transform skyboxTransform;
            skyboxTransform.position = levelScene->cameras[playerIndex].transform.position;
            quatIdent(&skyboxTransform.rotation);
            skyboxTransform.scale = gOneVec;
            transformToMatrixL(&skyboxTransform, skyboxMatrix);
            gSPMatrix(renderState->dl++, skyboxMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
            gSPDisplayList(renderState->dl++, levelScene->definition->theme->skyboxMaterial);
            gSPDisplayList(renderState->dl++, levelScene->definition->theme->skybox);
            gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
            gDPPipeSync(renderState->dl++);
        }
        

        gSPSetGeometryMode(renderState->dl++, G_ZBUFFER | G_CULL_BACK);
        gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
        gSPSegment(renderState->dl++, MATRIX_TRANSFORM_SEGMENT, levelScene->decorMatrices);
        gSPDisplayList(renderState->dl++, levelScene->levelDL);
        gSPDisplayList(renderState->dl++, baseGfx);
        gSPDisplayList(renderState->dl++, playerGfx);
        gSPDisplayList(renderState->dl++, minionGfx);
        gSPDisplayList(renderState->dl++, itemDropsGfx);
        gSPDisplayList(renderState->dl++, renderState->transparentQueueStart);

        gSPDisplayList(renderState->dl++, mat_Scramblers_f3d_material);
        for (unsigned playerIndexB = 0; playerIndexB < levelScene->playerCount; ++playerIndexB) {
            controlsScramblerRender(&levelScene->scramblers[playerIndexB], &levelScene->players[playerIndexB], renderState);
        }
        gSPDisplayList(renderState->dl++, mat_revert_Scramblers_f3d_material);

        baseCommandMenuRender(
            &levelScene->baseCommandMenu[playerIndex], 
            renderState, 
            clippingRegions
        );
        playerStatusMenuRender(
            &levelScene->players[playerIndex], 
            renderState, 
            levelScene->winningTeam, 
            levelScene->knockoutTimer < LOSE_BY_KNOCKOUT_TIME ? levelScene->knockoutTimer : -1.0f, 
            clippingRegions
        );

        ++humanIndex;
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

    gfxDrawTimingInfo(renderState);

    spriteFinish(renderState);
}

void leveSceneUpdateCamera(struct LevelScene* levelScene, unsigned playerIndex) {
    struct Vector3 target = levelScene->players[playerIndex].transform.position;
    vector3AddScaled(&target, &gUp, 2.0f * SCENE_SCALE, &target);

    if (levelScene->cameras[playerIndex].mode == 0) {
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

    cameraSetIsMapView(&levelScene->cameras[playerIndex], controllerGetButton(playerIndex, R_TRIG));

    Vp* viewport = &gSplitScreenViewports[0];
    float aspectRatio = (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1];

    cameraUpdate(&levelScene->cameras[playerIndex], &target, 15.0f * SCENE_SCALE, 5.0f * SCENE_SCALE, aspectRatio);

    struct Vector2 camPos2d;
    camPos2d.x = levelScene->cameras[playerIndex].transform.position.x;
    camPos2d.y = levelScene->cameras[playerIndex].transform.position.z;
    staticSceneConstrainToBoundaries(&levelScene->definition->staticScene, &camPos2d, 0, 0.5f * SCENE_SCALE);
    levelScene->cameras[playerIndex].transform.position.x = camPos2d.x;
    levelScene->cameras[playerIndex].transform.position.z = camPos2d.y;
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
            0,
            // controlsScramblerIsActive(&levelScene->scramblers[playerIndex], ControlsScramblerDPADSwap) ? PlayerInputFlagsSwapJoystickAndDPad : 0,
            playerInput
        );
    }
}

void levelSceneCollectPlayerInput(struct LevelScene* levelScene, unsigned playerIndex, unsigned botIndex, struct PlayerInput* playerInput) {
    if (levelScene->state == LevelSceneStatePlaying) {
        if (!IS_PLAYER_AI(levelScene, playerIndex)) {
            levelSceneCollectHumanPlayerInput(levelScene, playerIndex, playerInput);
        } else if (!(levelScene->levelFlags & LevelMetadataFlagsTutorial)) {
            ai_collectPlayerInput(levelScene, &levelScene->bots[botIndex], playerInput);
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

void levelSceneDeathSFX_Trigger(struct LevelScene* levelScene){

    for(unsigned i = 0; i < levelScene->playerCount; ++i){
        if(levelScene->deadPlayers[i] == 1){
            if(!playerIsAlive(&levelScene->players[i]))return;
            else levelScene->deadPlayers[i] = 0;
        }
    }

    for(unsigned i = 0; i < levelScene->playerCount; ++i){
        if(!playerIsAlive(&levelScene->players[i]) && !IS_PLAYER_AI(levelScene, i)){
            levelScene->deadPlayers[i] = 1;
            soundPlayerPlay(SOUNDS_DEATHSFX, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
        }
    }
}

void levelSceneUpdate(struct LevelScene* levelScene) {
    if (levelScene->state == LevelSceneStatePaused) {
        unsigned togglePause = 0;

        for (unsigned playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
            if (!IS_PLAYER_AI(levelScene, playerIndex) && controllerGetButtonDown(playerIndex, START_BUTTON)) {
                soundPlayerPlay(SOUNDS_UI_SELECT2, 0.5f, SoundPlayerPriorityNonPlayer, 0, 0);
                togglePause = 1;
                break;
            }
        }

        if (togglePause) {
            textBoxHide(&gTextBox);
        }

        if (!textBoxIsVisible(&gTextBox)) {
            levelScene->state = LevelSceneStatePlaying;
        }

        textBoxUpdate(&gTextBox);
        
        return;
    }

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

    levelSceneDeathSFX_Trigger(levelScene);

    unsigned botIndex = 0;
    unsigned humanIndex = 0;
    unsigned togglePause = 0;

    for (unsigned playerIndex = 0; playerIndex < levelScene->playerCount; ++playerIndex) {
        struct PlayerInput* playerInput = &levelScene->scramblers[playerIndex].playerInput;

        controlsScramblerUpdate(&levelScene->scramblers[playerIndex]);

        levelSceneCollectPlayerInput(levelScene, playerIndex, botIndex, playerInput);

        soundPlayerUpdateListener(humanIndex, &levelScene->cameras[playerIndex].transform.position, &levelScene->cameras[playerIndex].transform.rotation);

        if (IS_PLAYER_AI(levelScene, playerIndex)) {
            ++botIndex;
        } else {
            ++humanIndex;
            if (controllerGetButtonDown(playerIndex, START_BUTTON)) {
                soundPlayerPlay(SOUNDS_UI_SELECT2, 0.5f, SoundPlayerPriorityNonPlayer, 0, 0);
                togglePause = 1;
            }
        }

        if (!playerIsAlive(&levelScene->players[playerIndex])) {
            baseCommandMenuHide(&levelScene->baseCommandMenu[playerIndex]);
        }

        controlsScramblerApply(&levelScene->scramblers[playerIndex], IS_PLAYER_AI(levelScene, playerIndex));

        baseCommandMenuUpdate(&levelScene->baseCommandMenu[playerIndex], playerIndex);
        playerUpdate(&levelScene->players[playerIndex], playerInput);
        leveSceneUpdateCamera(levelScene, playerIndex);

        if (levelScene->state == LevelSceneStatePlaying && levelScene->levelFlags & (LevelMetadataFlagsTutorial | LevelMetadataFlagsTutorial2)) {
            tutorialUpdate(levelScene, playerInput);
        }
    }

    if (togglePause && levelScene->state == LevelSceneStatePlaying) {
        levelScene->state = LevelSceneStatePaused;
        textBoxInit(&gTextBox, "Paused", 200, SCREEN_WD / 2, SCREEN_HT / 2);
    }

    if (levelScene->state == LevelSceneStateIntro) {
        levelScene->stateTimer -= gTimeDelta;

        if (levelScene->stateTimer < 0.0f) {
            levelScene->stateTimer = GO_SHOW_DURATION;
            levelScene->state = LevelSceneStatePlaying;
            textBoxInit(&gTextBox, "GO!", 200, SCREEN_WD / 2, SCREEN_HT / 2);
            gTextBox.currState = TextBoxStateShowing;
            gTextBox.animateTimer = 0.0f;
        } else if (levelScene->stateTimer < GAME_START_DELAY * 0.5f) {
            textBoxHide(&gTextBox);
        }

        textBoxUpdate(&gTextBox);
        return;
    } else if (levelScene->state == LevelSceneStatePlaying && levelScene->stateTimer > 0) {
        levelScene->stateTimer -= gTimeDelta;

        if (levelScene->stateTimer <= 0.0f) {
            levelScene->stateTimer = 0.0f;
            textBoxHide(&gTextBox);
        }
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

    if (!(levelScene->levelFlags & LevelMetadataFlagsDisableItems)) {
        itemDropsUpdate(&levelScene->itemDrops);
    }

    dynamicSceneCollide();
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

void levelBaseDespawnMinions(struct LevelScene* levelScene, unsigned char baseId, unsigned newTeam) {
    for (unsigned i = 0; i < levelScene->minionCount; ++i) {
        if (minionIsAlive(&levelScene->minions[i]) && levelScene->minions[i].sourceBaseId == baseId && levelScene->minions[i].team.teamNumber != newTeam) {
            minionApplyDamage(&levelScene->minions[i], 100.0f, &levelScene->bases[i].position, 0.0f);
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

#if WIN_BY_PRESSING_L
    for (unsigned i = 0; i < levelScene->playerCount; ++i) {
        if (controllerGetButtonDown(i, L_TRIG)) {
            return i;
        }
    }
#endif

    int result = -1;

    if (levelScene->humanPlayerCount == 1 && !playerIsAlive(&levelScene->players[0]) && levelScene->players[0].controlledBases == 0) {
        result = 1;
        for (unsigned i = 2; i < levelScene->playerCount; ++i) {
            if (levelScene->players[i].controlledBases > levelScene->players[result].controlledBases) {
                result = i;
            }
        }
        return result;
    }

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

void levelSceneApplyScrambler(struct LevelScene* levelScene, unsigned toTeam, enum ControlsScramblerType scambler) {
    if (playerIsAlive(&levelScene->players[toTeam])) {
        controlsScramblerTrigger(&levelScene->scramblers[toTeam], scambler);
    }
}