#ifndef _OPTIONS_MENU_H
#define _OPTIONS_MENU_H

#include "graphics/render_state.h"

struct OptionsMenu {
    unsigned short itemSelection;
    unsigned short isErasing;
};

void optionsMenuInit(struct OptionsMenu* optionsMenu);
int optionsMenuUpdate(struct OptionsMenu* optionsMenu);
void optionsMenuRender(struct OptionsMenu* optionsMenu, struct RenderState* renderState);

#endif