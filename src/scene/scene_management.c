#include "scene_management.h"
#include "levels/levels.h"
#include "graphics/gfx.h"
#include "util/rom.h"
#include "util/memory.h"
#include "menu/endgamemenu.h"
#include "savefile/savefile.h"
#include "menu/credits.h"
#include "levels/themedefinition.h"
#include "menu/spinninglogo.h"
#include "audio/soundplayer.h"

enum SceneState gSceneState;
enum SceneState gNextSceneState;
struct LevelScene gCurrentLevel;
struct MainMenu gMainMenu;
struct Credits gCredits;
struct GameConfiguration gNextLevel;

enum SceneState gAfterCutscene;
static enum CutsceneIndex gNextCutscene;
unsigned frameSkip = 0;

extern char _staticSegmentRomStart[], _staticSegmentRomEnd[];


int sceneIsLoading() {
    if (frameSkip) {
        --frameSkip;
        return 1;
    }

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

    levelSceneInit(&gCurrentLevel, definition, gameConfig->playerCount, gameConfig->aiPlayerMask, metadata->flags, sceneIsCampaign() ? definition->aiDifficulty : 1.0f);
    gSceneState = SceneStateInLevel;
}

void sceneQueueLoadLevel(struct GameConfiguration* nextLevel) {
    gNextLevel = *nextLevel;
    gNextSceneState = SceneStateInLevel;
}

void sceneQueueCredits() {
    gNextSceneState = SceneStateInCredits;
}

void sceneQueueIntro() {
    gNextSceneState = SceneStateIntro;
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

    if (winningTeam == 0 && sceneIsCampaign()) {
        saveFileMarkLevelComplete(gMainMenu.selections.selectedLevel, time);
        ++gMainMenu.selections.selectedLevel;
    }
}

void sceneInsertCutscene(enum CutsceneIndex cutsceneIndex) {
    gAfterCutscene = gNextSceneState;
    gNextCutscene = cutsceneIndex;
    gNextSceneState = SceneStateInCutscene;
}

void sceneLoadMainMenu() {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(mainmenu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(fonts, gFontSegment);
    mainMenuInit(&gMainMenu);
    gSceneState = SceneStateInMainMenu;
}

void sceneLoadCredits() {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(mainmenu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(fonts, gFontSegment);
    creditsInit(&gCredits);
    gSceneState = SceneStateInCredits;
}

void sceneLoadIntro() {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(mainmenu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(fonts, gFontSegment);
    spinningLogoInit(&gSpinningLogo);
    gSceneState = SceneStateIntro;
}

extern char _SpaceSegmentRomStart[], _SpaceSegmentRomEnd[];
extern char _cutscene_setSegmentRomStart[], _cutscene_setSegmentRomEnd[];

void sceneLoadCutscene() {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(gameplaymenu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(fonts, gFontSegment);
    LOAD_SEGMENT(cutscene_set, gLevelSegment);

    gThemeSegment = malloc(_SpaceSegmentRomEnd - _SpaceSegmentRomStart);
    romCopy(_SpaceSegmentRomStart, gThemeSegment, _SpaceSegmentRomEnd - _SpaceSegmentRomStart);

    gSceneState = SceneStateInCutscene;
    cutsceneInit(&gCutscene, gNextCutscene);
}

void sceneEndCutscene() {
    gNextSceneState = gAfterCutscene;
}

int sceneIsCampaign() {
    return gMainMenu.selections.selectedPlayerCount == 0;
}

void sceneCleanup() {
    gLevelSegment = 0;
    gThemeSegment = 0;
    heapReset();
    skResetDataPool();
    soundPlayerReset();
}

void sceneUpdate(int readyForSceneSwitch) {
    if (sceneIsLoading()) {
        if (readyForSceneSwitch && !skHasPendingMessages()) {
            sceneCleanup();
            switch (gNextSceneState) {
                case SceneStateInLevel:
                    sceneLoadLevel(&gNextLevel);
                    break;
                case SceneStateIntro:
                    sceneLoadIntro();
                    break;
                case SceneStateInMainMenu:
                    sceneLoadMainMenu();
                    break;
                case SceneStateInCredits:
                    sceneLoadCredits();
                    break;
                case SceneStateInCutscene:
                    sceneLoadCutscene();
                    break;
                default:
                    break;
            }
            frameSkip = 1;
        }
    } else {
        switch (gSceneState) {
            case SceneStateInLevel:
                levelSceneUpdate(&gCurrentLevel);
                break;
            case SceneStateIntro:
                spinningLogoUpdate(&gSpinningLogo);
                break;
            case SceneStateInMainMenu:
                mainMenuUpdate(&gMainMenu);
                break;
            case SceneStateInCredits:
                creditsUpdate(&gCredits);
                break;
            case SceneStateInCutscene:
                cutsceneUpdate(&gCutscene);
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
        case SceneStateIntro:
            spinningLogoRender(&gSpinningLogo, renderState);
            break;
        case SceneStateInMainMenu:
            mainMenuRender(&gMainMenu, renderState);
            break;
        case SceneStateInCredits:
            creditsRender(&gCredits, renderState);
            break;
        case SceneStateInCutscene:
            cutsceneRender(&gCutscene, renderState);
            break;
        default:
            break;
    }
}