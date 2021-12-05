#ifndef LEVEL_SELECTION_H
#define LEVEL_SELECTION_H

#include "graphics/render_state.h"

struct MainMenu;

void mainMenuEnterLevelSelection(struct MainMenu* mainMenu);
void mainMenuUpdateLevelSelect(struct MainMenu* mainMenu);
void mainMenuRenderLevels(struct MainMenu* mainMenu, struct RenderState* renderState);

#endif