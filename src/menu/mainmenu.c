#include "mainmenu.h"
#include "graphics/spritefont.h"
#include "graphics/image.h"
#include "graphics/gfx.h"

#include "../data/mainmenu/menu.h"
#include "../data/models/characters.h"

void mainMenuInit(struct MainMenu* mainMenu) {
    cameraInit(&mainMenu->camera, 450.0f, 100.0f, 18000.0f);
    mainMenu->camera.transform.position.z = 10000.0f;
}

void mainMenuUpdate(struct MainMenu* mainMenu) {

}

void mainMenuRender(struct MainMenu* mainMenu, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);

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
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);

    // gSPDisplayList(renderState->dl++, Minion_DogMinion_mesh);
    gSPDisplayList(renderState->dl++, Mars_Mars_mesh);
    gDPPipeSync(renderState->dl++);

    // spriteSolid(renderState, LAYER_SOLID_COLOR, 0, 0, 200, 200);

    graphicsCopyImage(renderState, players_1_img, 64, 77, 0, 0, 68, 34, 64, 77, 255);
    graphicsCopyImage(renderState, players_2_img, 64, 77, 0, 0, 177, 34, 64, 77, 255);
    graphicsCopyImage(renderState, players_3_img, 64, 77, 0, 0, 68, 131, 64, 77, 255);
    graphicsCopyImage(renderState, players_4_img, 64, 77, 0, 0, 177, 131, 64, 77, 255);

    spriteFinish(renderState);
}