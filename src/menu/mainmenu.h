#ifndef _MAIN_MENU_H
#define _MAIN_MENU_H

#include "graphics/render_state.h"

struct MainMenu {

};

void mainMenuInit(struct MainMenu* mainMenu);
void mainMenuUpdate(struct MainMenu* mainMenu);
void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState);

#endif