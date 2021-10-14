
#include "render_state.h"
#include <assert.h>

void renderStateInit(struct RenderState* renderState) {
    renderState->dl = renderState->glist;
    renderState->currentMatrix = 0;
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
    assert((void *)renderState->dl <= (void *)&renderState->glist[MAX_DL_LENGTH]);
    osWritebackDCache(renderState->glist, (s32)renderState->dl - (s32)renderState->glist);
    osWritebackDCache(renderState->matrices, sizeof(Mtx) * renderState->currentMatrix);
}