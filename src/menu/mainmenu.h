#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include "graphics/render_state.h"
#include "scene/camera.h"

enum MainMenuState {
    MainMenuStateSelectingPlayerCount,
    MainMenuStateSelectingLevel,
};

struct MainMenu {
    struct Camera camera;
    struct Transform marsTransform;
    enum MainMenuState menuState;
    unsigned short selectedPlayerCount;
    unsigned short selectedLevel;
    struct LevelMetadata** filteredLevels;
    unsigned short levelCount;
};

void mainMenuInit(struct MainMenu* mainMenu);
void mainMenuUpdate(struct MainMenu* mainMenu);
void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState);

#endif