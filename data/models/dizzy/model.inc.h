Lights1 Dizzy_Dizzy_lights = gdSPDefLights1(
	0x7F, 0x7F, 0x7F,
	0xFE, 0xFE, 0xFE, 0x28, 0x28, 0x28);

u64 Dizzy_Dizzy_Face_ci4[] = {
	0x0000000012221000, 0x0000000013333100, 0x0000000133313100, 0x0000001333333110, 0x0000113333311331, 0x1110133333114131, 0x2221333331114413, 0x2133111331444413, 
	0x2131444133144131, 0x2314444413311331, 0x1314444413312133, 0x3314411113112213, 0x1331411131112213, 0x0133111312221133, 0x0013333311221331, 0x0000131333113310, 
	
};

u64 Dizzy_Dizzy_Face_ci4_pal_rgba16[] = {
	0xfffe318d6c5db769, 0xffff000000000000
};

Vtx Dizzy_Dizzy_mesh_vtx_0[4] = {
	{{{-10, -10, 0},0, {-16, 496},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{10, -10, 0},0, {496, 496},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{10, 10, 0},0, {496, -16},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{-10, 10, 0},0, {-16, -16},{0x0, 0x0, 0x7F, 0xFF}}},
};

Gfx Dizzy_Dizzy_mesh_tri_0[] = {
	gsSPVertex(Dizzy_Dizzy_mesh_vtx_0 + 0, 4, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSP1Triangle(0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_Dizzy_Dizzy[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, Dizzy_Dizzy_Face_ci4_pal_rgba16),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 4),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_16b, 1, Dizzy_Dizzy_Face_ci4),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 4, 0, G_TX_CLAMP | G_TX_NOMIRROR, 4, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 63, 2048),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 4, 0, G_TX_CLAMP | G_TX_NOMIRROR, 4, 0),
	gsDPSetTileSize(0, 0, 0, 60, 60),
	gsSPSetLights1(Dizzy_Dizzy_lights),
	gsSPEndDisplayList(),
};

Gfx mat_revert_Dizzy_Dizzy[] = {
	gsDPPipeSync(),
	gsDPSetTextureLUT(G_TT_NONE),
	gsSPEndDisplayList(),
};

Gfx Dizzy_Dizzy_mesh[] = {
	gsSPDisplayList(mat_Dizzy_Dizzy),
	gsSPDisplayList(Dizzy_Dizzy_mesh_tri_0),
	gsSPDisplayList(mat_revert_Dizzy_Dizzy),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

