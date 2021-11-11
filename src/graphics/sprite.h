
#ifndef _SPRITE_H
#define _SPRITE_H

#include <ultra64.h>
#include "render_state.h"
#include "math/color.h"

#define LAYER_SOLID_COLOR       0
#define LAYER_COMMAND_BUTTONS   3
#define LAYER_UPGRADE_ICONS     4
#define LAYER_GB_FONT           5
#define LAYER_KICKFLIP_FONT     6

struct SpriteTile
{
    char x;
    char y;
    char w;
    char h;
};

void spriteWriteRaw(struct RenderState* renderState, int layer, Gfx* src, int count);
void spriteSetLayer(struct RenderState* renderState, int layer, Gfx* graphics);
void spriteSolid(struct RenderState* renderState, int layer, int x, int y, int w, int h);
void spriteDraw(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int scaleShiftX, int scaleShiftY);
void spriteDrawTile(struct RenderState* renderState, int layer, int x, int y, int w, int h, struct SpriteTile tile);
void spriteSetColor(struct RenderState* renderState, int layer, struct Coloru8 color);

void spriteInit(struct RenderState* renderState);
void spriteFinish(struct RenderState* renderState);

#endif