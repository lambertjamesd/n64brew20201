#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include "graphics/render_state.h"
#include "scene/camera.h"
#include "sk64/skelatool_animator.h"
#include "sk64/skelatool_armature.h"
#include "game_defs.h"

enum MainMenuState {
    MainMenuStateSelectingPlayerCount,
    MainMenuStateSelectingFaction,
    MainMenuStateSelectingLevel,
};

enum MainMenuFactionFlags {
    MainMenuFactionFlagsAI = (1 << 0),
    MainMenuFactionFlagsSelected = (1 << 0),
};

struct MainMenuFactionSelector {
    struct SKAnimator animator;
    struct SKArmature armature;
    struct Transform transform;
    unsigned short selectedFaction;
    unsigned short flags;
    float rotateLerp;
};

struct MainMenu {
    struct Camera camera;
    struct Transform marsTransform;
    enum MainMenuState menuState;
    unsigned short selectedPlayerCount;
    unsigned short selectedLevel;
    struct LevelMetadata** filteredLevels;
    unsigned short levelCount;
    struct MainMenuFactionSelector factionSelection[MAX_PLAYERS];
};

void mainMenuInit(struct MainMenu* mainMenu);
void mainMenuUpdate(struct MainMenu* mainMenu);
void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState);

#endif