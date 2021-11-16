#ifndef MENU_BORDER_H
#define MENU_BORDER_H

#include "graphics/render_state.h"

#define MENU_BORDER_WIDTH  6

void menuBorderRender(struct RenderState* renderState, int x, int y, int w, int h);

#endif