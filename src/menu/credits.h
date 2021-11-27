#ifndef _CREDITS_H
#define _CREDITS_H

#include "graphics/render_state.h"

struct Credits{
    float offset;
};

void creditsInit(struct Credits* credits);
void creditsUpdate(struct Credits* credits);
void creditsRender(struct Credits* credits, struct RenderState* renderState);

#endif