#include "mainmenu.h"
#include "graphics/spritefont.h"
#include "graphics/image.h"

#include "../data/mainmenu/menu.h"

void mainMenuInit(struct MainMenu* mainMenu) {
    
}

void mainMenuUpdate(struct MainMenu* mainMenu) {

}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);

    // spriteSolid(renderState, LAYER_SOLID_COLOR, 0, 0, 200, 200);

    graphicsCopyImage(renderState, players_1_img, 64, 77, 0, 0, 32, 32, 64, 77, 255);

    spriteFinish(renderState);
}