#ifndef _RENDER_STATE_H
#define _RENDER_STATE_H

#include <ultra64.h>


#define MAX_ACTIVE_TRANSFORMS   320
#define MAX_DL_LENGTH           1024

struct RenderState {
    Gfx glist[MAX_DL_LENGTH];
    Mtx matrices[MAX_ACTIVE_TRANSFORMS];
    Gfx* dl;
    unsigned currentMatrix;
    unsigned currentChunkEnd;
};

void renderStateInit(struct RenderState* renderState);
Mtx* renderStateRequestMatrices(struct RenderState* renderState, unsigned count);
void renderStateFlushCache(struct RenderState* renderState);
Gfx* renderStateAllocateDLChunk(struct RenderState* renderState, unsigned count);
Gfx* renderStateReplaceDL(struct RenderState* renderState, Gfx* nextDL);

#endif