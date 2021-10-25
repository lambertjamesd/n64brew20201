
#include "geometry_geo.inc.h"

Lights1 doglow_ArmorBlue_lights = gdSPDefLights1(
	0x7F, 0x7F, 0x7F,
	0xFE, 0xFE, 0xFE, 0x28, 0x28, 0x28);

u64 doglow_TeamPallete_ci4[] = {
	0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 
	0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 
};

u64 doglow_TeamPallete_ci4_pal_rgba16[] = {
	0x3efd2991a21395ef, 
};

Vtx doglow_DogLow_mesh_vtx_cull[8] = {
	{{{-111, 0, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-111, 0, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-111, 238, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-111, 238, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 0, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 0, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 238, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 238, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
};

Gfx mat_doglow_ArmorBlue[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535 >> 1, 65535 >> 1, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, doglow_TeamPallete_ci4_pal_rgba16),
    gsDPSetTextureFilter(G_TF_AVERAGE),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 3),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_16b, 1, doglow_TeamPallete_ci4),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 63, 2048),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPSetTileSize(0, 0, 0, 60, 60),
	gsSPSetLights1(doglow_ArmorBlue_lights),
	gsSPEndDisplayList(),
};

Gfx mat_revert_doglow_ArmorBlue[] = {
	gsDPPipeSync(),
	gsDPSetTextureLUT(G_TT_NONE),
	gsSPEndDisplayList(),
};

Gfx doglow_DogLow_mesh[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(doglow_DogLow_mesh_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_doglow_ArmorBlue),
	gsSPDisplayList(doglow_model_gfx),
	gsSPDisplayList(mat_revert_doglow_ArmorBlue),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

