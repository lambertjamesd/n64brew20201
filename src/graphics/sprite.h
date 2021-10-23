
#ifndef _SPRITE_H
#define _SPRITE_H

#include <ultra64.h>
#include "render_state.h"

#define LAYER_C_BUTTONS         0
#define LAYER_COMMAND_BUTTONS   1

struct SpriteTile
{
    char x;
    char y;
    char w;
    char h;
};

void spriteSetLayer(struct RenderState* renderState, int layer, Gfx* graphics);
void spriteDraw(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int scaleShiftX, int scaleShiftY);
void spriteDrawTile(struct RenderState* renderState, int layer, int x, int y, int w, int h, struct SpriteTile tile);
void spriteSetColor(struct RenderState* renderState, int layer, u8 r, u8 g, u8 b, u8 a);

void spriteInit(struct RenderState* renderState);
void spriteFinish(struct RenderState* renderState);

#endif