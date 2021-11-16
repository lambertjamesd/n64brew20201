
#include "sprite.h"
#include "assert.h"
#include "gfx.h"
#include "game_defs.h"
#include "util/time.h"
#include "image.h"

#define DL_CHUNK_SIZE       32

void spriteWriteRaw(struct RenderState* renderState, int layer, Gfx* src, int count)
{
    while (count)
    {
        Gfx* current = renderState->spriteState.currentLayerDL[layer];
        int capacity = DL_CHUNK_SIZE + renderState->spriteState.layerChunk[layer] - current;

        if (!current || capacity == 1)
        {
            Gfx* next = renderStateAllocateDLChunk(renderState, DL_CHUNK_SIZE);

            if (current)
            {
                // check if the next chunk is adjacent in memory
                if (current + 1 == next)
                {
                    *current++ = *src++;
                    --count;
                    --capacity;
                }
                else
                {
                    gSPBranchList(current++, next);
                }
            }
            else
            {
                renderState->spriteState.layerDL[layer] = next;
            }

            renderState->spriteState.layerChunk[layer] = next;
            renderState->spriteState.currentLayerDL[layer] = next;
            current = next;
        }

        while (count && capacity > 1)
        {
            *current++ = *src++;
            --count;
            --capacity;
        }

        renderState->spriteState.currentLayerDL[layer] = current;
    }
}

void spriteSetLayer(struct RenderState* renderState, int layer, Gfx* graphics) {
    renderState->spriteState.layerSetup[layer] = graphics;
}

void spriteSolid(struct RenderState* renderState, int layer, int x, int y, int w, int h) {
    Gfx workingMem[4];
    Gfx* curr = workingMem;
    gDPFillRectangle(curr++, x, y, x + w, y + h);
    spriteWriteRaw(renderState, layer, workingMem, curr - workingMem);
    
}

void spriteCopyImage(struct RenderState* renderState, int layer, void* image, int iw, int ih, int x, int y, int w, int h, int sx, int sy) {
    Gfx* start = renderStateStartChunk(renderState);
    graphicsCopyImage(renderState, image, iw, ih, sx, sy, x, y, w, h, renderState->spriteState.layerColor[layer]);
    Gfx* chunk = renderStateEndChunk(renderState, start);
    Gfx tmp[1];
    Gfx* tmpPtr = tmp;
    gSPDisplayList(tmpPtr++, chunk);
    spriteWriteRaw(renderState, layer, tmp, tmpPtr - tmp);
}


void spriteTextureRectangle(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int dsdx, int dsdy) {
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    gSPTextureRectangle(
        curr++,
        x,
        y,
        x + w,
        y + h,
        G_TX_RENDERTILE,
        sx,
        sy,
        dsdx,
        dsdy
    );

    spriteWriteRaw(renderState, layer, workingMem, curr - workingMem);
}

void spriteDraw(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    unsigned dsdx = 0x400;
    unsigned dtdy = 0x400;

    if (sw >= 0) {
        w <<= sw;
        dsdx >>= sw;
    } else {
        w >>= -sw;
        dsdx <<= -sw;
    }

    if (sh >= 0) {
        h <<= sh;
        dtdy >>= sh;
    } else {
        h >>= -sh;
        dtdy <<= -sh;
    }

    gSPTextureRectangle(
        curr++,
        x << 2, 
        y << 2,
        (x + w) << 2,
        (y + h) << 2,
        G_TX_RENDERTILE,
        sx << 5, sy << 5,
        dsdx,
        dtdy
    );

    spriteWriteRaw(renderState, layer, workingMem, curr - workingMem);
}

void spriteDrawTile(struct RenderState* renderState, int layer, int x, int y, int w, int h, struct SpriteTile tile)
{
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    gSPTextureRectangle(
        curr++,
        x << 2, 
        y << 2,
        (x + w) << 2,
        (y + h) << 2,
        G_TX_RENDERTILE,
        tile.x << 5, tile.y << 5,
        (tile.w << 10) / w,
        (tile.h << 10) / h
    );

    spriteWriteRaw(renderState, layer, workingMem, curr - workingMem);
}

void spriteSetColor(struct RenderState* renderState, int layer, struct Coloru8 color)
{
    struct Coloru8 currColor = renderState->spriteState.layerColor[layer];
    if (color.r != currColor.r || color.g != currColor.g || color.b != currColor.b || color.a != currColor.a)
    {
        Gfx workingMem[2];
        Gfx* curr = workingMem;
        gDPPipeSync(curr++);
        gDPSetEnvColor(curr++, color.r, color.g, color.b, color.a);
        spriteWriteRaw(renderState, layer, workingMem, curr - workingMem);
        renderState->spriteState.layerColor[layer] = color;
    }
}

void spriteInit(struct RenderState* renderState)
{
    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        renderState->spriteState.layerDL[i] = 0;
        renderState->spriteState.currentLayerDL[i] = 0;
        renderState->spriteState.layerChunk[i] = 0;
        renderState->spriteState.layerColor[i] = gColorWhite;
    }
}

void spriteFinish(struct RenderState* renderState)
{
    Mtx* menuMatrices = renderStateRequestMatrices(renderState, 2);

    if (!menuMatrices) {
        return;
    }

    guOrtho(&menuMatrices[0], 0, SCREEN_WD, SCREEN_HT, 0, -SCENE_SCALE, SCENE_SCALE, 1.0f);
    guRotate(&menuMatrices[1], -90.0f, 1.0f, 0.0f, 0.0f);

    gSPMatrix(renderState->dl++, &menuMatrices[0], G_MTX_PROJECTION | G_MTX_NOPUSH | G_MTX_LOAD);
    gSPMatrix(renderState->dl++, &menuMatrices[1], G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_LOAD);

    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        if (renderState->spriteState.layerDL[i] && (renderState->spriteState.layerSetup[i] || i == LAYER_IMAGE_COPIES))
        {
            gSPEndDisplayList(renderState->spriteState.currentLayerDL[i]++);
            if (renderState->spriteState.layerSetup[i]) {
                gSPDisplayList(renderState->dl++, renderState->spriteState.layerSetup[i]);
            }
            gSPDisplayList(renderState->dl++, renderState->spriteState.layerDL[i]);
        }
    }
}
