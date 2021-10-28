#ifndef _MINIMAP_H
#define _MINIMAP_H

#include "graphics/render_state.h"

struct LevelScene;

void minimapRender(struct LevelScene* scene, struct RenderState* renderState, unsigned short* minimapLocation);

#endif