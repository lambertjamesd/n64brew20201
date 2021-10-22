
#include "sprite.h"
#include "assert.h"

#define DL_CHUNK_SIZE       32

void writeDLToSprite(struct RenderState* renderState, int layer, Gfx* src, int count)
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

void spriteSetLayer(struct RenderState* renderState, int layer, Gfx* graphics)
{
    renderState->spriteState.layerSetup[layer] = graphics;
}

void spriteDraw(struct RenderState* renderState, int layer, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
    Gfx workingMem[4];
    Gfx* curr = workingMem;

    gSPTextureRectangle(
        curr++,
        x << 2, 
        y << 2,
        (x + (w << sw)) << 2,
        (y + (h << sh)) << 2,
        G_TX_RENDERTILE,
        sx << 5, sy << 5,
        0x400 >> sw,
        0x400 >> sh
    );

    writeDLToSprite(renderState, layer, workingMem, curr - workingMem);
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

    writeDLToSprite(renderState, layer, workingMem, curr - workingMem);
}

void spriteSetColor(struct RenderState* renderState, int layer, u8 r, u8 g, u8 b, u8 a)
{
    int key = (r << 24) | (g << 16) | (b << 8) | (a << 0);

    if (key != renderState->spriteState.layerColor[layer])
    {
        Gfx workingMem;
        Gfx* curr = &workingMem;
        gDPSetEnvColor(curr++, r, g, b, a);
        writeDLToSprite(renderState, layer, &workingMem, curr - &workingMem);
        renderState->spriteState.layerColor[layer] = key;
    }
}

void spriteInit(struct RenderState* renderState)
{
    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        renderState->spriteState.layerDL[i] = 0;
        renderState->spriteState.currentLayerDL[i] = 0;
        renderState->spriteState.layerChunk[i] = 0;
        renderState->spriteState.layerColor[i] = ~0;
    }
}

void spriteFinish(struct RenderState* renderState)
{
    for (int i = 0; i < MAX_LAYER_COUNT; ++i)
    {
        if (renderState->spriteState.layerDL[i] && renderState->spriteState.layerSetup[i])
        {
            gSPEndDisplayList(renderState->spriteState.currentLayerDL[i]++);
            gSPDisplayList(renderState->dl++, renderState->spriteState.layerSetup[i]);
            gSPDisplayList(renderState->dl++, renderState->spriteState.layerDL[i]);
        }
    }
}
