
#include "render_state.h"
#include <assert.h>
#include "sprite.h"

void renderStateInit(struct RenderState* renderState) {
    renderState->dl = renderState->glist;
    renderState->currentMatrix = 0;
    renderState->currentChunkEnd = MAX_DL_LENGTH;
    renderState->transparentQueueStart = renderStateAllocateDLChunk(renderState, TRANSPARENT_QUEUE_LEN);
    renderState->transparentDL = renderState->transparentQueueStart;
    
    gDPSetRenderMode(renderState->transparentDL++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
    spriteInit(renderState);
}

Mtx* renderStateRequestMatrices(struct RenderState* renderState, unsigned count) {
    if (renderState->currentMatrix + count <= MAX_ACTIVE_TRANSFORMS) {
        Mtx* result = &renderState->matrices[renderState->currentMatrix];
        renderState->currentMatrix += count;
        return result;
    }

    return 0;
}

void renderStateFlushCache(struct RenderState* renderState) {
    assert((void *)renderState->dl <= (void *)&renderState->glist[renderState->currentChunkEnd]);
    assert((void *)renderState->transparentDL <= (void *)&renderState->transparentQueueStart[TRANSPARENT_QUEUE_LEN]);
    osWritebackDCache(renderState->glist, sizeof(renderState->glist));
    osWritebackDCache(renderState->matrices, sizeof(Mtx) * renderState->currentMatrix);
}

Gfx* renderStateAllocateDLChunk(struct RenderState* renderState, unsigned count) {
    Gfx* result = &renderState->glist[renderState->currentChunkEnd - count];
    assert(result >= renderState->dl);
    renderState->currentChunkEnd -= count;
    return result;
}

Gfx* renderStateReplaceDL(struct RenderState* renderState, Gfx* nextDL) {
    Gfx* result = renderState->dl;
    renderState->dl = nextDL;
    return result;
}

Gfx* renderStateStartChunk(struct RenderState* renderState) {
    return renderState->dl;    
}

Gfx* renderStateEndChunk(struct RenderState* renderState, Gfx* chunkStart) {
    Gfx* newChunk = renderStateAllocateDLChunk(renderState, (renderState->dl - chunkStart) + 1);
    Gfx* copyDest = newChunk;
    Gfx* copySrc = chunkStart;

    while (copySrc < renderState->dl) {
        *copyDest = *copySrc;
        ++copyDest;
        ++copySrc;
    }

    gSPEndDisplayList(copyDest++);

    renderState->dl = chunkStart;

    return newChunk;
}