#include "scene_management.h"
#include "levels/level_test.h"
#include "levels/levels.h"
#include "graphics/gfx.h"
#include "util/rom.h"
#include "util/memory.h"
#include "menu/endgamemenu.h"
#include "savefile/savefile.h"

enum SceneState gSceneState;
enum SceneState gNextSceneState;
struct LevelScene gCurrentLevel;
struct MainMenu gMainMenu;
struct GameConfiguration gNextLevel;

struct LevelDefinition* gLevelsTmp[] = {
    &gLevelTest,
};

extern char _staticSegmentRomStart[], _staticSegmentRomEnd[];


int sceneIsLoading() {
    return gSceneState != gNextSceneState;
}

void sceneLoadLevel(struct GameConfiguration* gameConfig) {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(gameplaymenu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(fonts, gFontSegment);

    struct LevelMetadata* metadata = gameConfig->level;

    gLevelSegment = malloc(metadata->romSegmentEnd - metadata->romSegmentStart);
    romCopy(metadata->romSegmentStart, gLevelSegment, metadata->romSegmentEnd - metadata->romSegmentStart);

    gThemeSegment = malloc(metadata->theme->romSegmentEnd - metadata->theme->romSegmentStart);
    romCopy(metadata->theme->romSegmentStart, gThemeSegment, metadata->theme->romSegmentEnd - metadata->theme->romSegmentStart);

    struct LevelDefinition* definition = levelDefinitionUnpack(metadata->fullDefinition, gLevelSegment, gThemeSegment);

    levelSceneInit(&gCurrentLevel, definition, gameConfig->playerCount, gameConfig->humanPlayerCount, metadata->flags);
    gSceneState = SceneStateInLevel;
}

void sceneQueueLoadLevel(struct GameConfiguration* nextLevel) {
    gNextLevel = *nextLevel;
    gNextSceneState = SceneStateInLevel;
}

void sceneQueueMainMenu() {
    gNextSceneState = SceneStateInMainMenu;
    gMainMenu.selections.menuState = MainMenuStateSelectingTitleScreen;
    gMainMenu.selections.targetMenuState = MainMenuStateSelectingTitleScreen;
}

void sceneQueuePostGameScreen(unsigned winningTeam, unsigned teamCount, float time) {
    gNextSceneState = SceneStateInMainMenu;
    endGameMenuInit(&gMainMenu.endGameMenu, winningTeam, teamCount, time);
    gMainMenu.selections.menuState = MainMenuStatePostGame;
    gMainMenu.selections.targetMenuState = MainMenuStatePostGame;

    if (winningTeam == 0 && gMainMenu.selections.selectedPlayerCount == 0) {
        saveFileMarkLevelComplete(gMainMenu.selections.selectedLevel, time);
        ++gMainMenu.selections.selectedLevel;
    }
}

void sceneLoadMainMenu() {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(mainmenu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(fonts, gFontSegment);
    mainMenuInit(&gMainMenu);
    gSceneState = SceneStateInMainMenu;
}

void sceneCleanup() {
    gLevelSegment = 0;
    gThemeSegment = 0;
    heapReset();
    skResetDataPool();
}

void sceneUpdate(int readyForSceneSwitch) {
    if (sceneIsLoading()) {
        if (readyForSceneSwitch && !skHasPendingMessages()) {
            sceneCleanup();
            switch (gNextSceneState) {
                case SceneStateInLevel:
                    sceneLoadLevel(&gNextLevel);
                    break;
                case SceneStateInMainMenu:
                    sceneLoadMainMenu();
                    break;
                default:
                    break;
            }
        }
    } else {
        switch (gSceneState) {
            case SceneStateInLevel:
                levelSceneUpdate(&gCurrentLevel);
                break;
            case SceneStateInMainMenu:
                mainMenuUpdate(&gMainMenu);
                break;
            default:
                break;
        }
    }
}

void sceneRender(struct RenderState* renderState) {
    switch (gSceneState) {
        case SceneStateInLevel:
            levelSceneRender(&gCurrentLevel, renderState);
            break;
        case SceneStateInMainMenu:
            mainMenuRender(&gMainMenu, renderState);
        default:
            break;
    }
}