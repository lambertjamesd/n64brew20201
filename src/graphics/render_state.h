#ifndef _RENDER_STATE_H
#define _RENDER_STATE_H

#include <ultra64.h>
#include "math/color.h"
#include "math/quaternion.h"

#define MAX_ACTIVE_TRANSFORMS   320
#define MAX_DL_LENGTH           3000
#define TRANSPARENT_QUEUE_LEN   300
#define MAX_LAYER_COUNT     8

struct SpriteState {
    Gfx* layerSetup[MAX_LAYER_COUNT];
    struct Coloru8 layerColor[MAX_LAYER_COUNT];
    Gfx* layerDL[MAX_LAYER_COUNT];
    Gfx* currentLayerDL[MAX_LAYER_COUNT];
    Gfx* currentChunkEnd[MAX_LAYER_COUNT];
};

struct RenderState {
    Gfx glist[MAX_DL_LENGTH];
    Mtx matrices[MAX_ACTIVE_TRANSFORMS];
    Gfx* dl;
    Gfx* transparentQueueStart;
    Gfx* transparentDL;
    unsigned currentMatrix;
    unsigned currentChunkEnd;
    struct SpriteState spriteState;
    struct Quaternion* cameraRotation;
};

void renderStateInit(struct RenderState* renderState);
Mtx* renderStateRequestMatrices(struct RenderState* renderState, unsigned count);
void renderStateFlushCache(struct RenderState* renderState);
Gfx* renderStateAllocateDLChunk(struct RenderState* renderState, unsigned count);
Gfx* renderStateReplaceDL(struct RenderState* renderState, Gfx* nextDL);
Gfx* renderStateStartChunk(struct RenderState* renderState);
Gfx* renderStateEndChunk(struct RenderState* renderState, Gfx* chunkStart);

#endif