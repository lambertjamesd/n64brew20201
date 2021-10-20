
#include "render_state.h"
#include <assert.h>

void renderStateInit(struct RenderState* renderState) {
    renderState->dl = renderState->glist;
    renderState->currentMatrix = 0;
    renderState->currentChunkEnd = MAX_DL_LENGTH;
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
    osWritebackDCache(renderState->glist, (s32)renderState->dl - (s32)renderState->glist);
    osWritebackDCache(renderState->matrices, sizeof(Mtx) * renderState->currentMatrix);
}

Gfx* renderStateAllocateDLChunk(struct RenderState* renderState, unsigned count) {
    Gfx* result = &renderState->glist[renderState->currentChunkEnd - count];
    assert(result >= renderState->dl);
    return result;
}

Gfx* renderStateReplaceDL(struct RenderState* renderState, Gfx* nextDL) {
    Gfx* result = renderState->dl;
    renderState->dl = nextDL;
    return result;
}