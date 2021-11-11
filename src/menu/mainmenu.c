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

#include "../data/mainmenu/menu.h"
#include "../data/models/characters.h"

#define MARS_ROTATE_RATE    (2.0f * M_PI / 30.0f)
#define MARS_TILT           (-10.0f * M_PI / 180.0f)

#define CYCLE_TIME          2.0f

struct Coloru8 gDeselectedColor = {128, 128, 128, 255};
struct Coloru8 gMenuBlue = {127, 174, 177, 255};
struct Coloru8 gMenuBlack = {41, 41, 35, 255};

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

void mainMenuStartLevel(struct MainMenu* mainMenu) {
    struct GameConfiguration gameConfig;
    gameConfig.humanPlayerCount = mainMenu->selectedPlayerCount + 1;

    if (gameConfig.humanPlayerCount == 1) {
        gameConfig.playerCount = 2;
    } else {
        gameConfig.playerCount = gameConfig.humanPlayerCount;
    }

    gameConfig.level = mainMenu->filteredLevels[mainMenu->selectedLevel];

    sceneQueueLoadLevel(&gameConfig);
}

void mainMenuEnterLevelSelection(struct MainMenu* mainMenu) {
    mainMenu->menuState = MainMenuStateSelectingLevel;
    
    mainMenu->selectedLevel = 0;
    mainMenu->levelCount = 0;

    for (unsigned i = 0; i < gLevelCount; ++i) {
        if (gLevels[i].flags & LevelMetadataFlagsMultiplayer) {
            mainMenu->filteredLevels[mainMenu->levelCount] = &gLevels[i];
            ++mainMenu->levelCount;
        }
    }
}

void mainMenuInit(struct MainMenu* mainMenu) {
    cameraInit(&mainMenu->camera, 22.0f, 100.0f, 18000.0f);
    mainMenu->camera.transform.position.z = 600.0f;
    transformInitIdentity(&mainMenu->marsTransform);
    mainMenu->marsTransform.position.x = 50.0f;
    mainMenu->menuState = MainMenuStateSelectingPlayerCount;
    mainMenu->selectedPlayerCount = 0;
    mainMenu->selectedLevel = 0;
    mainMenu->filteredLevels = malloc(sizeof(struct ThemeMetadata*) * gLevelCount);
    initKickflipFont();
}

void mainMenuUpdatePlayerCount(struct MainMenu* mainMenu) {
    enum ControllerDirection direction = controllerGetDirectionDown(0);
    
    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selectedPlayerCount > 0) {
        --mainMenu->selectedPlayerCount;
    }
    if ((direction & ControllerDirectionRight) != 0 && mainMenu->selectedPlayerCount + 1 < MAX_PLAYERS) {
        ++mainMenu->selectedPlayerCount;
    }
    if ((direction & ControllerDirectionDown) != 0 && mainMenu->selectedPlayerCount + 2 < MAX_PLAYERS) {
        mainMenu->selectedPlayerCount += 2;
    }
    if ((direction & ControllerDirectionUp) != 0 && mainMenu->selectedPlayerCount > 1) {
        mainMenu->selectedPlayerCount -= 2;
    }

    if (controllerGetButtonDown(0, A_BUTTON)) {
        mainMenuEnterLevelSelection(mainMenu);
    }
}

void mainMenuUpdateLevelSelect(struct MainMenu* mainMenu) {

    enum ControllerDirection direction = controllerGetDirectionDown(0);

    if ((direction & ControllerDirectionLeft) != 0 && mainMenu->selectedLevel > 0) {
        --mainMenu->selectedLevel;
    }

    if ((direction & ControllerDirectionRight) != 0 && mainMenu->selectedLevel + 1 < mainMenu->levelCount) {
        ++mainMenu->selectedLevel;
    }

    if (controllerGetButtonDown(0, A_BUTTON)) {
        mainMenuStartLevel(mainMenu);
    }

    if (controllerGetButtonDown(0, B_BUTTON)) {
        mainMenu->menuState = MainMenuStateSelectingPlayerCount;
    }
}

void mainMenuUpdate(struct MainMenu* mainMenu) {
    struct Quaternion axisSpin;
    struct Quaternion axisTilt;
    quatAxisAngle(&gUp, gTimePassed * MARS_ROTATE_RATE, &axisSpin);
    quatAxisAngle(&gForward, MARS_TILT, &axisTilt);
    quatMultiply(&axisTilt, &axisSpin, &mainMenu->marsTransform.rotation);

    switch (mainMenu->menuState) {
        case MainMenuStateSelectingPlayerCount:
            mainMenuUpdatePlayerCount(mainMenu);
            break;
        case MainMenuStateSelectingLevel:
            mainMenuUpdateLevelSelect(mainMenu);
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
        if (mainMenu->selectedPlayerCount == i) {
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

void mainMenuRenderLevels(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetColor(renderState, LAYER_SOLID_COLOR, gMenuBlue);
    spriteSolid(renderState, LAYER_SOLID_COLOR, 59, 28, 200, 36);
    spriteSetColor(renderState, LAYER_SOLID_COLOR, gMenuBlack);
    spriteSolid(renderState, LAYER_SOLID_COLOR, 66, 30, 186, 34);

    char* name = mainMenu->filteredLevels[mainMenu->selectedLevel]->name;

    unsigned x = (SCREEN_WD - fontMeasure(&gKickflipFont, name, 0)) >> 1;

    fontRenderText(renderState, &gKickflipFont, name, x, 38, 0);
}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);

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
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    Mtx* marsMatrix = renderStateRequestMatrices(renderState, 1);
    transformToMatrixL(&mainMenu->marsTransform, marsMatrix);
    gSPMatrix(renderState->dl++, marsMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, Mars_Mars_mesh);
    gSPPopMatrix(renderState->dl++, 1);

    gDPPipeSync(renderState->dl++);

    switch (mainMenu->menuState) {
        case MainMenuStateSelectingPlayerCount:
            mainMenuRenderPlayerCount(mainMenu, renderState);
            break;
        case MainMenuStateSelectingLevel:
            mainMenuRenderLevels(mainMenu, renderState);
            break;
    }

    spriteFinish(renderState);
}