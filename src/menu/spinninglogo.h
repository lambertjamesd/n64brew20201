#ifndef _SPINNING_LOGO_H
#define _SPINNING_LOGO_H

#include "graphics/render_state.h"
#include "scene/camera.h"

struct SpinningLogo {
    float timer;
    struct Camera camera;
};

extern struct SpinningLogo gSpinningLogo;

void spinningLogoInit(struct SpinningLogo* spinningLogo);
void spinningLogoUpdate(struct SpinningLogo* spinningLogo);
void spinningLogoRender(struct SpinningLogo* spinningLogo, struct RenderState* renderState);

#endif