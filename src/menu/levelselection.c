#include "levelselection.h"

#include "mainmenu.h"
#include "graphics/gfx.h"
#include "levels/levels.h"
#include "../data/mainmenu/menu.h"
#include "textbox.h"
#include "audio/clips.h"
#include "audio/soundplayer.h"
#include "controls/controller.h"
#include "util/time.h"
#include "kickflipfont.h"
#include "savefile/savefile.h"
#include "util/rom.h"
#include "graphics/sprite.h"
#include "graphics/image.h"

int mainMenuIsLevelUnlocked(struct MainMenu* mainMenu) {
    return mainMenu->selections.selectedLevel < mainMenu->unlockedLevelCount;
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
    mainMenu->unlockedLevelCount = 1;
    unsigned playercount = mainMenuGetPlayerCount(mainMenu);
    unsigned campaignIndex = 0;

    for (unsigned i = 0; i < gLevelCount; ++i) {
        if ((playercount > 1 && (gLevels[i].flags & LevelMetadataFlagsMultiplayer) != 0 && gLevels[i].maxPlayers >= playercount) || 
            (playercount == 1 && (gLevels[i].flags & LevelMetadataFlagsCampaign) != 0)) {
            mainMenu->filteredLevels[mainMenu->levelCount] = &gLevels[i];
            ++mainMenu->levelCount;

            if ((gLevels[i].flags & LevelMetadataFlagsCampaign) == 0 || saveFileIsLevelComplete(campaignIndex)) {
                ++mainMenu->unlockedLevelCount;
            }
        }

        if (gLevels[i].flags & LevelMetadataFlagsCampaign) {
            ++campaignIndex;
        }
    }

    if (mainMenu->selections.selectedLevel >= mainMenu->levelCount) {
        mainMenu->selections.selectedLevel = mainMenu->levelCount - 1;
    }

    if (mainMenu->selections.selectedLevel >= mainMenu->unlockedLevelCount) {
        mainMenu->selections.selectedLevel = mainMenu->unlockedLevelCount - 1;
    }

    textBoxInit(&gTextBox, mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->name, 200, SCREEN_WD / 2, 46);
    mainMenuLoadWireframe(mainMenu, &mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->wireframe);
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

    if ((direction & ControllerDirectionRight) != 0 && 
        mainMenu->selections.selectedLevel < mainMenu->unlockedLevelCount && 
        mainMenu->selections.selectedLevel + 1 < mainMenu->levelCount) {
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

    if (controllerGetButtonDown(0, A_BUTTON) && mainMenuIsLevelUnlocked(mainMenu)) {
        soundPlayerPlay(SOUNDS_UI_SELECT2, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
        mainMenu->selections.targetMenuState = MainMenuStateStarting;
        textBoxHide(&gTextBox);
    }

    if (controllerGetButtonDown(0, B_BUTTON)) {
        soundPlayerPlay(SOUNDS_UI_SELECT3, 1.0f, 1.0f, SoundPlayerPriorityPlayer, 0, 0);
        mainMenu->selections.targetMenuState = MainMenuStateSelectingFaction;
        textBoxHide(&gTextBox);
    }
}

void mainMenuRenderLevels(struct MainMenu* mainMenu, struct RenderState* renderState) {
    textBoxRender(&gTextBox, renderState);

    if (mainMenu->showWireframeDelay) {
        --mainMenu->showWireframeDelay;

        if (mainMenu->showWireframeDelay == 0) {
            mainMenuLoadWireframe(mainMenu, &mainMenu->filteredLevels[mainMenu->selections.selectedLevel]->wireframe);
        }
    }

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

    graphicsCopyImage(renderState, ArrowButtons_0_0, 32, 64, 0, 16 + (mainMenu->selections.selectedLevel == 0 ? 32 : 0), 24, SCREEN_HT / 2 - 8, 16, 16, gColorWhite);
    graphicsCopyImage(renderState, ArrowButtons_0_0, 32, 64, 16, 16 + (mainMenu->selections.selectedLevel + 1 >= mainMenu->levelCount ? 32 : 0), SCREEN_WD - 40, SCREEN_HT / 2 - 8, 16, 16, gColorWhite);

    if (mainMenu->selections.selectedPlayerCount == 0 && saveFileIsLevelComplete(mainMenu->selections.selectedLevel)) {
        char timeString[16];
        formatTimeString(saveFileLevelTime(mainMenu->selections.selectedLevel), timeString);
        unsigned timeWidth = fontMeasure(&gKickflipFont, timeString, 0);
        fontRenderText(renderState, &gKickflipFont, timeString, 260 - timeWidth / 2, 200, 0);
    }
}