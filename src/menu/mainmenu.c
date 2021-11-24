#include "mainmenu.h"
#include "graphics/spritefont.h"
#include "graphics/image.h"
#include "graphics/gfx.h"
#include "util/time.h"
#include "math/mathf.h"
#include "game_defs.h"
#include "scene/team_data.h"
#include "controls/controller.h"
#include "scene/scene_management.h"
#include "kickflipfont.h"
#include "util/memory.h"
#include "scene/faction.h"
#include "util/rom.h"
#include "textbox.h"
#include "savefile/savefile.h"

#include "../data/mainmenu/menu.h"
#include "../data/fonts/fonts.h"
#include "../data/models/characters.h"

#define MARS_ROTATE_RATE    (2.0f * M_PI / 30.0f)
#define MARS_TILT           (-10.0f * M_PI / 180.0f)

#define CYCLE_TIME              2.0f

#define SELECT_ANGLE            (M_PI * 1.2f)
#define SELECT_SPIN_TIME        0.5f
#define UNSELECTED_SPIN_FREQ    (0.2f * (M_PI * 2.0f))

struct Coloru8 gDeselectedColor = {128, 128, 128, 255};

struct ButtonLayoutInfo {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned short* imgData;
};

struct ButtonLayoutInfo gPlayerCountSelectButtons[] = {
    {68, 34, 64, 77, players_1_img},
    {177, 34, 64, 77, players_2_img},
    {68, 131, 64, 77, players_3_img},
    {177, 131, 64, 77, players_4_img},
};

int mainMenuGetPlayerCount(struct MainMenu* menu) {
    return menu->selections.selectedPlayerCount + 1;
}

int mainMenuIsLevelUnlocked(struct MainMenu* mainMenu) {
    if (mainMenu->selections.selectedPlayerCount > 0) {
        return 1;
    }

    return mainMenu->selections.selectedLevel == 0 || saveFileIsLevelComplete(mainMenu->selections.selectedLevel - 1);
}

void mainMenuFactionInit(struct MainMenuFactionSelector* faction, unsigned index) {
    faction->selectedFaction = 0;

    for (unsigned i = 0; i < FACTION_COUNT; ++i) {
        if (gTeamFactions[index] == gFactions[i]) {
            faction->selectedFaction = i;
            break;
        }
    }

    skAnimatorInit(&faction->animator, ANY_FACTION_BONE_COUNT, 0, 0);
    skArmatureInit(
        &faction->armature, 
        gFactions[faction->selectedFaction]->playerMesh, 
        ANY_FACTION_BONE_COUNT, 
        CALC_ROM_POINTER(character_animations, gFactions[faction->selectedFaction]->playerDefaultPose), 
        gFactions[0]->playerBoneParent
    );
    faction->flags = 0;
    transformInitIdentity(&faction->transform);
    vector3Scale(&gOneVec, &faction->transform.scale, 0.5f);
    faction->transform.position.y = -50.0f;

    skAnimatorRunClip(
        &faction->animator, 
        gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelectIdle], 
        SKAnimatorFlagsLoop
    );

    faction->rotateLerp = 0.0f;
}

void mainMenuFactionUpdate(struct MainMenuFactionSelector* faction, unsigned index) {
    skAnimatorUpdate(&faction->animator, faction->armature.boneTransforms, 1.0f);

    if ((faction->flags & MainMenuFactionFlagsSelected) == 0) {
        enum ControllerDirection controllerDirection = controllerGetDirectionDown(index);

        unsigned prevFaction = faction->selectedFaction;

        if (controllerDirection & ControllerDirectionLeft) {
            if (faction->selectedFaction == 0) {
                faction->selectedFaction = FACTION_COUNT - 1;
            } else {
                --faction->selectedFaction;
            }
        }

        if (controllerDirection & ControllerDirectionRight) {
            ++faction->selectedFaction;

            if (faction->selectedFaction >= FACTION_COUNT) {
                faction->selectedFaction = 0;
            }
        }

        faction->rotateLerp = mathfMoveTowards(faction->rotateLerp, 0.0f, gTimeDelta / SELECT_SPIN_TIME);
        
        if (controllerGetButtonDown(index, A_BUTTON)) {
            soundPlayerPlay(SOUNDS_UI_SELECT, 0);
            faction->flags |= MainMenuFactionFlagsSelected;
            skAnimatorRunClip(
                &faction->animator, 
                gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelected], 
                0
            );
        } else if (prevFaction != faction->selectedFaction) {
            faction->armature.displayList = gFactions[faction->selectedFaction]->playerMesh;
            skAnimatorRunClip(
                &faction->animator, 
                gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelectIdle], 
                SKAnimatorFlagsLoop
            );
        }
    } else {
        if (controllerGetButtonDown(index, B_BUTTON)) {
            soundPlayerPlay(SOUNDS_UI_SELECT, 0);
            faction->flags &= ~MainMenuFactionFlagsSelected;

            skAnimatorRunClip(
                &faction->animator, 
                gFactions[faction->selectedFaction]->playerAnimations[PlayerAnimationSelectIdle], 
                SKAnimatorFlagsLoop
            );
        }

        faction->rotateLerp = mathfMoveTowards(faction->rotateLerp, 1.0f, gTimeDelta / SELECT_SPIN_TIME);
    }

    float rotateAngle = mathfMod(gTimePassed * UNSELECTED_SPIN_FREQ + M_PI, M_PI * 2.0f);

    if (faction->rotateLerp <= 0.0f) {
        quatAxisAngle(&gUp, rotateAngle, &faction->transform.rotation);
    } else if (faction->rotateLerp >= 1.0f) {
        quatAxisAngle(&gUp, SELECT_ANGLE, &faction->transform.rotation);
    } else {
        quatAxisAngle(&gUp, mathfLerp(rotateAngle, SELECT_ANGLE, faction->rotateLerp), &faction->transform.rotation);
    }
}
 
void mainMenuStartLevel(struct MainMenu* mainMenu) {
    struct GameConfiguration gameConfig;
    gameConfig.humanPlayerCount = mainMenuGetPlayerCount(mainMenu);

    if (gameConfig.humanPlayerCount == 1) {
        gameConfig.playerCount = mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->maxPlayers;
    } else {
        gameConfig.playerCount = gameConfig.humanPlayerCount;
    }

    gameConfig.level = mainMenu->filteredLevels[mainMenu->selections.selectedLevel];

    for (unsigned i = 0; i <= mainMenu->selections.selectedPlayerCount; ++i) {
        gTeamFactions[i] = gFactions[mainMenu->factionSelection[i].selectedFaction];
    }

    sceneQueueLoadLevel(&gameConfig);
}

void mainMenuEnterFactionSelection(struct MainMenu* mainMenu) {
    mainMenu->selections.menuState = MainMenuStateSelectingFaction;
    mainMenu->selections.targetMenuState = MainMenuStateSelectingFaction;

    gfxInitSplitscreenViewport(mainMenuGetPlayerCount(mainMenu));

    for (unsigned i = 0; i < mainMenuGetPlayerCount(mainMenu); ++i) {
        mainMenu->factionSelection->flags = controllerIsConnected(i) ? 0 : MainMenuFactionFlagsAI;
    }
}

void mainMenuLoadWireframe(struct MainMenu* mainMenu, struct WireframeMetadata* wireframe) {
    if (mainMenuIsLevelUnlocked(mainMenu)) {
        romCopy(wireframe->romSegmentStart, gWireframeSegment, wireframe->romSegmentEnd - wireframe->romSegmentStart);
        mainMenu->showingWireframe = wireframe->wireframe;
    } else {
        mainMenu->showingWireframe = gLevelplaceholder_mesh;
    }
}

void mainMenuEnterLevelSelection(struct MainMenu* mainMenu) {
    mainMenu->selections.menuState = MainMenuStateSelectingLevel;
    mainMenu->selections.targetMenuState = MainMenuStateSelectingLevel;
    
    mainMenu->levelCount = 0;
    unsigned playercount = mainMenuGetPlayerCount(mainMenu);

    for (unsigned i = 0; i < gLevelCount; ++i) {
        if ((playercount > 1 && (gLevels[i].flags & LevelMetadataFlagsMultiplayer) != 0 && gLevels[i].maxPlayers >= playercount) || 
            (playercount == 1 && (gLevels[i].flags & LevelMetadataFlagsCampaign) != 0)) {
            mainMenu->filteredLevels[mainMenu->levelCount] = &gLevels[i];
            ++mainMenu->levelCount;
        }
    }

    if (mainMenu->selections.selectedLevel >= mainMenu->levelCount) {
        mainMenu->selections.selectedLevel = mainMenu->levelCount - 1;
    }

    textBoxInit(&gTextBox, mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->name, 200, SCREEN_WD / 2, 46);
    mainMenuLoadWireframe(mainMenu, &mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->wireframe);
}

void mainMenuInitSelections(struct MainMenu* mainMenu) {
    mainMenu->selections.menuState = MainMenuStateSelectingPlayerCount;
    mainMenu->selections.targetMenuState = MainMenuStateSelectingPlayerCount;

    mainMenu->selections.selectedPlayerCount = 0;
    mainMenu->selections.selectedLevel = 0;
}

void mainMenuInit(struct MainMenu* mainMenu) {
    cameraInit(&mainMenu->camera, 22.0f, 100.0f, 18000.0f);
    mainMenu->camera.transform.position.z = 600.0f;
    transformInitIdentity(&mainMenu->marsTransform);
    mainMenu->marsTransform.position.x = 50.0f;
    mainMenu->filteredLevels = malloc(sizeof(struct ThemeMetadata*) * gLevelCount);
    mainMenu->showingWireframe = 0;
    mainMenu->showWireframeDelay = 0;
    initKickflipFont();

    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        mainMenuFactionInit(&mainMenu->factionSelection[i], i);
    }

    unsigned wireframeSize = 0;

    for (unsigned i = 0; i < gLevelCount; ++i) {
        wireframeSize = MAX(wireframeSize, gLevels[i].wireframe.romSegmentEnd - gLevels[i].wireframe.romSegmentStart);
    }

    gWireframeSegment = malloc(wireframeSize);
}

void mainMenuUpdatePlayerCount(struct MainMenu* mainMenu) {
    enum ControllerDirection direction = controllerGetDirectionDown(0);
    
    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selections.selectedPlayerCount > 0) {
        --mainMenu->selections.selectedPlayerCount;
    }
    if ((direction & ControllerDirectionRight) != 0 && mainMenuGetPlayerCount(mainMenu) < MAX_PLAYERS) {
        ++mainMenu->selections.selectedPlayerCount;
    }
    if ((direction & ControllerDirectionDown) != 0 && mainMenu->selections.selectedPlayerCount + 2 < MAX_PLAYERS) {
        mainMenu->selections.selectedPlayerCount += 2;
    }
    if ((direction & ControllerDirectionUp) != 0 && mainMenu->selections.selectedPlayerCount > 1) {
        mainMenu->selections.selectedPlayerCount -= 2;
    }

    if (controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenuEnterFactionSelection(mainMenu);
    }
}

void mainMenuUpdateFaction(struct MainMenu* mainMenu) {
    unsigned isReady = 1;

    if ((mainMenu->factionSelection[0].flags & MainMenuFactionFlagsSelected) == 0 && controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenu->selections.menuState = MainMenuStateSelectingPlayerCount;
    }

    for (unsigned i = 0; i <= mainMenu->selections.selectedPlayerCount; ++i) {
        if (!(mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected)) {
            isReady = 0;
        }

        mainMenuFactionUpdate(&mainMenu->factionSelection[i], i);
    }

    if (isReady && controllerGetButtonDown(0, A_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenuEnterLevelSelection(mainMenu);
    }
}

void mainMenuUpdateLevelSelect(struct MainMenu* mainMenu) {
    textBoxUpdate(&gTextBox);

    if (mainMenu->selections.targetMenuState == MainMenuStateStarting) {
        if (!textBoxIsVisible(&gTextBox)) {
            mainMenuStartLevel(mainMenu);
        }
        return;
    } else if (mainMenu->selections.targetMenuState == MainMenuStateSelectingFaction) {
        if (!textBoxIsVisible(&gTextBox)) {
            mainMenu->selections.menuState = MainMenuStateSelectingFaction;
            mainMenu->selections.targetMenuState = MainMenuStateSelectingFaction;
        }
        return;
    }

    enum ControllerDirection direction = controllerGetDirectionDown(0);

    unsigned lastLevel = mainMenu->selections.selectedLevel;

    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selections.selectedLevel > 0) {
        --mainMenu->selections.selectedLevel;
    }

    if ((direction & ControllerDirectionRight) != 0 && mainMenu->selections.selectedLevel + 1 < mainMenu->levelCount) {
        ++mainMenu->selections.selectedLevel;
    }

    if (lastLevel != mainMenu->selections.selectedLevel) {
        mainMenu->showingWireframe = 0;
        // delay 2 frames to ensure any pending display lists
        // are done with gWireframeSegment before changing it
        mainMenu->showWireframeDelay = 5;
        if (mainMenuIsLevelUnlocked(mainMenu)) {
            textBoxChangeText(&gTextBox, mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->name);
        } else {
            textBoxChangeText(&gTextBox, "???");
        }
    }

    if (mainMenu->showWireframeDelay) {
        --mainMenu->showWireframeDelay;

        if (mainMenu->showWireframeDelay == 0) {
            mainMenuLoadWireframe(mainMenu, &mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->wireframe);
        }
    }

    if (controllerGetButtonDown(0, A_BUTTON) && mainMenuIsLevelUnlocked(mainMenu)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenu->selections.targetMenuState = MainMenuStateStarting;
        textBoxHide(&gTextBox);
    }

    if (controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT, 0);
        mainMenu->selections.targetMenuState = MainMenuStateSelectingFaction;
        textBoxHide(&gTextBox);
    }
}

void mainMenuUpdate(struct MainMenu* mainMenu) {
    struct Quaternion axisSpin;
    struct Quaternion axisTilt;
    quatAxisAngle(&gUp, gTimePassed * MARS_ROTATE_RATE, &axisSpin);
    quatAxisAngle(&gForward, MARS_TILT, &axisTilt);
    quatMultiply(&axisTilt, &axisSpin, &mainMenu->marsTransform.rotation);

    switch (mainMenu->selections.menuState) {
        case MainMenuStateSelectingPlayerCount:
            mainMenuUpdatePlayerCount(mainMenu);
            break;
        case MainMenuStateSelectingFaction:
            mainMenuUpdateFaction(mainMenu);
            break;
        case MainMenuStateSelectingLevel:
            mainMenuUpdateLevelSelect(mainMenu);
            break;
        case MainMenuStateStarting:
            break;
        case MainMenuStatePostGame:
            if (endGameMenuUpdate(&mainMenu->endGameMenu)) {
                mainMenuEnterLevelSelection(mainMenu);
            }
            break;
    }
}

void mainMenuSelectionColor(struct Coloru8* result) {
    float timeLerp = mathfMod(gTimePassed, CYCLE_TIME) * (MAX_PLAYERS / CYCLE_TIME);
    int colorIndex = ((int)timeLerp) % MAX_PLAYERS;
    int nextColor = (colorIndex + 1) % MAX_PLAYERS;
    colorU8Lerp(&gTeamColors[colorIndex], &gTeamColors[nextColor], timeLerp - colorIndex, result);
    colorU8Lerp(result, &gColorWhite, 0.5f, result);
}

void mainMenuRenderPlayerCount(struct MainMenu* mainMenu, struct RenderState* renderState) {
    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        struct Coloru8 color;
        if (mainMenu->selections.selectedPlayerCount == i) {
            mainMenuSelectionColor(&color);
        } else {
            color = gDeselectedColor;
        }

        struct ButtonLayoutInfo* button = &gPlayerCountSelectButtons[i];

        graphicsCopyImage(
            renderState,
            button->imgData,
            button->w,
            button->h,
            0, 0,
            button->x,
            button->y,
            button->w,
            button->h,
            color
        );
    }
}

void mainMenuRenderFactions(struct MainMenu* mainMenu, struct RenderState* renderState) {
    unsigned isReady = 1;

    for (unsigned int i = 0; i < mainMenu->selections.selectedPlayerCount+1; ++i) {
        gDPPipeSync(renderState->dl++);
        Vp* viewport = &gSplitScreenViewports[i];
        cameraSetupMatrices(
            &mainMenu->camera, 
            renderState, 
            (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1],
            0.0f
        );
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
        
        Mtx* playerMatrix = renderStateRequestMatrices(renderState, 1);
        transformToMatrixL(&mainMenu->factionSelection[i].transform, playerMatrix);
        gSPMatrix(renderState->dl++, playerMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPDisplayList(renderState->dl++, gTeamPalleteTexture[i]);
        skRenderObject(&mainMenu->factionSelection[i].armature, renderState);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

        if (!(mainMenu->factionSelection[i].flags & MainMenuFactionFlagsSelected)) {
            isReady = 0;
        }
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

    if (isReady) {
        spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
        spriteSolid(renderState, LAYER_SOLID_COLOR, 0, 88, SCREEN_WD, 64);

        char* message = "Ready?";
        unsigned messageX = (SCREEN_WD - fontMeasure(&gKickflipFont, message, 1)) >> 1;
        fontRenderText(renderState, &gKickflipFont, message, messageX, 104, 1);
    }
}

void mainMenuRenderLevels(struct MainMenu* mainMenu, struct RenderState* renderState) {
    textBoxRender(&gTextBox, renderState);

    if (mainMenu->showingWireframe) {
        Mtx* matrix = renderStateRequestMatrices(renderState, 1);
        struct Transform transform;
        transform.position = gZeroVec;
        transform.position.y = -0.3f * SCENE_SCALE;
        struct Quaternion topSpin;
        struct Quaternion sideTilt;

        quatAxisAngle(&gUp, gTimePassed, &topSpin);
        quatAxisAngle(&gRight, M_PI * 0.125f, &sideTilt);
        quatMultiply(&sideTilt, &topSpin, &transform.rotation);

        vector3Scale(&gOneVec, &transform.scale, 0.08f);

        transformToMatrixL(&transform, matrix);

        gSPMatrix(renderState->dl++, matrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPDisplayList(renderState->dl++, gMainMenuLevelWireframePass0);
        gSPDisplayList(renderState->dl++, mainMenu->showingWireframe);
        gSPDisplayList(renderState->dl++, gMainMenuLevelWireframePass1);
        gSPDisplayList(renderState->dl++, mainMenu->showingWireframe);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }

    if (mainMenu->selections.selectedPlayerCount == 0 && saveFileIsLevelComplete(mainMenu->selections.selectedLevel)) {
        char timeString[16];
        renderTimeString(saveFileLevelTime(mainMenu->selections.selectedLevel), timeString);
        unsigned timeWidth = fontMeasure(&gKickflipFont, timeString, 0);
        fontRenderText(renderState, &gKickflipFont, timeString, 260 - timeWidth / 2, 200, 0);
    }
}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);

    graphicsCopyImage(renderState, gMenuBackground, SCREEN_WD, SCREEN_HT, 0, 0, 0, 0, SCREEN_WD, SCREEN_HT, gColorWhite);
    gDPSetTexturePersp(renderState->dl++, G_TP_PERSP);

    cameraSetupMatrices(&mainMenu->camera, renderState, (float)SCREEN_WD/(float)SCREEN_HT, 0.0f);
    gSPViewport(renderState->dl++, osVirtualToPhysical(&gFullScreenVP));
    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE); 
    Mtx* marsMatrix = renderStateRequestMatrices(renderState, 1);
    transformToMatrixL(&mainMenu->marsTransform, marsMatrix);
    gSPMatrix(renderState->dl++, marsMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, Mars_Mars_mesh);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    gDPPipeSync(renderState->dl++);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    gDPSetRenderMode(renderState->dl++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);

    switch (mainMenu->selections.menuState) {
        case MainMenuStateSelectingPlayerCount:
            mainMenuRenderPlayerCount(mainMenu, renderState);
            break;
        case MainMenuStateSelectingFaction:
            mainMenuRenderFactions(mainMenu, renderState);
            break;
        case MainMenuStateSelectingLevel:
            mainMenuRenderLevels(mainMenu, renderState);
            break;
        case MainMenuStateStarting:
            break;
        case MainMenuStatePostGame:
            endGameMenuRender(&mainMenu->endGameMenu, renderState);
            break;
    }

    spriteFinish(renderState);
}