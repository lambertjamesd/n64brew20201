#ifndef _TITLE_SCREEN_H
#define _TITLE_SCREEN_H

#include "graphics/render_state.h"

struct TitleScreen {
    float timer;
    float targetTime;
};

void titleScreenInit(struct TitleScreen* titleScreen);
int titleScreenUpdate(struct TitleScreen* titleScreen);
void titleScreenRender(struct TitleScreen* titleScreen, struct RenderState* renderState);

#endif