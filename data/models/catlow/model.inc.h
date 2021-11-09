
#include "geometry_geo.inc.h"

Lights1 catlow_ArmorBlue_lights = gdSPDefLights1(
	0x7F, 0x7F, 0x7F,
	0xFE, 0xFE, 0xFE, 0x28, 0x28, 0x28);

u64 catlow_TeamPallete_ci4[] = {
	0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 
	0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 
};

u64 catlow_TeamPallete_ci4_pal_rgba16[] = {
	0x3efd2991a21395ef, 
};

Vtx catlow_CatLow_mesh_vtx_cull[8] = {
	{{{-111, 0, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-111, 0, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-111, 238, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-111, 238, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 0, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 0, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 238, 55},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{111, 238, -52},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
};

Gfx mat_catlow_ArmorBlue[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, SHADE, 0, 0, 0, 0, ENVIRONMENT),
	gsSPSetLights1(catlow_ArmorBlue_lights),
	gsSPEndDisplayList(),
};

Gfx mat_revert_catlow_ArmorBlue[] = {
	gsDPPipeSync(),
	gsDPSetTextureLUT(G_TT_NONE),
	gsSPEndDisplayList(),
};

Gfx catlow_CatLow_mesh[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(catlow_CatLow_mesh_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_catlow_ArmorBlue),
	gsSPDisplayList(catlow_model_gfx),
	gsSPDisplayList(mat_revert_catlow_ArmorBlue),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

