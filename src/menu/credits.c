#include "credits.h"
#include "kickflipfont.h"
#include "graphics/spritefont.h"
#include "graphics/image.h"
#include "graphics/gfx.h"
#include "controls/controller.h"
#include "scene/scene_management.h"
#include "util/time.h"

#include "../data/fonts/fonts.h"

#define SCROLL_SPEED 24

Gfx gCreditsSolidColor[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_LIGHTING | G_SHADE),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
	gsSPEndDisplayList(),
};

void creditsInit(struct Credits* credits) {
    credits->offset = -SCREEN_HT;
    initKickflipFont();
}

void creditsUpdate(struct Credits* credits) {
    if(controllerGetButtonDown(0, B_BUTTON)) sceneQueueCredits();

    if(!controllerGetButtonDown(0, A_BUTTON)){
        if(credits->offset >= SCREEN_HT*1.175) credits->offset = -SCREEN_HT;
        else credits->offset += SCROLL_SPEED * gTimeDelta;
    }
}

void drawTitle(struct RenderState* renderState, unsigned offset){
    char* title1 = "mission";
    char* title2 = "lost control";
    unsigned message1X = (SCREEN_WD - fontMeasure(&gKickflipFont, title1, 1)) >> 1;
    unsigned message2X = (SCREEN_WD - fontMeasure(&gKickflipFont, title2, 0)) >> 1;
    fontRenderText(renderState, &gKickflipFont, title1, message1X-24, 14 - offset, 1);
    fontRenderText(renderState, &gKickflipFont, title2, message2X+12, 45 - offset, 0);
}

void drawRoles(struct RenderState* renderState, unsigned offset){
    char* roles[] = {"Art and Graphics", "Music and SFX", "Programming"};

    for(unsigned i = 0; i < 3; ++i){
        fontRenderText(renderState, &gKickflipFont, roles[i], 36, (76 + 48*i) - offset, -1);
    }
}

void drawNames(struct RenderState* renderState, unsigned offset){
    char* names[] = {"@SapphireTactics", "@jtn191", "@lambertjamesd"};
    for(unsigned i = 0; i < 3; ++i){
        fontRenderText(renderState, &gKickflipFont, names[i], 39, (76+16 + 48*i) - offset, 0);
    }
    fontRenderText(renderState, &gKickflipFont, "whatswithandy", 39, (76+16+18 + 48*2) - offset, 0);
}

void drawGameJamNote(struct RenderState* renderState, unsigned offset){
    fontRenderText(renderState, &gKickflipFont, "made for", 36, (76+16+24 + 48*3) - offset, 0);
    fontRenderText(renderState, &gKickflipFont, "2021 n64brew", 36, (76+16+43 + 48*3) - offset, 0);
    fontRenderText(renderState, &gKickflipFont, "gamejam", 36, (76+16+61 + 48*3) - offset, 0);
}

void creditsRender(struct Credits* credits, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gCreditsSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);

    drawTitle(renderState, credits->offset);
    drawRoles(renderState, credits->offset);
    drawNames(renderState, credits->offset);
    drawGameJamNote(renderState, credits->offset);


    spriteFinish(renderState);
}