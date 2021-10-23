#include <ultra64.h>

#include "cbuttons.inc.h"
#include "commandicons.inc.h"

unsigned short __attribute__((aligned(8))) gGUIPalette[] = {
	0x0000,
	0xEF2B,
	0xEE87,
	0xBC83,
	0x1,
	0xC631,
	0x6B5D,
	0x2109,
	0x1,
	0xBE3F,
	0xCFB1,
	0x1935,
	0x1E05,
	0x2055,
	0x2C5,
	0xF5EF,
	0xC885,
	0x5045,
	0x1,
	0xEF2B,
	0xFFFF,
	0x0,
	0x1,
	0xC631,
	0x6B5D,
	0x2109,
};

Gfx gUseCButtons[] = {
    gsDPPipeSync(),
    gsDPSetCycleType(G_CYC_1CYCLE),
	gsSPClearGeometryMode(G_LIGHTING | G_SHADE | G_ZBUFFER),
    gsDPSetTexturePersp(G_TP_NONE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetEnvColor(255, 255, 255, 255),
	gsDPSetCombineLERP(TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0),
    gsDPSetTextureFilter(G_TF_POINT),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, gGUIPalette),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 26),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_8b, 32, tex_cbuttons),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_8b, 4, 0, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0),
	gsDPLoadSync(),
	gsDPLoadTile(7, 0, 0, 124, 124),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_8b, 4, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0),
	gsDPSetTileSize(0, 0, 0, 124, 124),
	gsSPEndDisplayList(),
};