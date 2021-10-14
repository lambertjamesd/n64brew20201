#ifndef _RENDER_STATE_H
#define _RENDER_STATE_H

#include <ultra64.h>


#define MAX_ACTIVE_TRANSFORMS   256
#define MAX_DL_LENGTH           512

struct RenderState {
    Gfx glist[MAX_DL_LENGTH];
    Mtx matrices[MAX_ACTIVE_TRANSFORMS];
    Gfx* dl;
    unsigned currentMatrix;
};

void renderStateInit(struct RenderState* renderState);
Mtx* renderStateRequestMatrices(struct RenderState* renderState, unsigned count);
void renderStateFlushCache(struct RenderState* renderState);

#endif