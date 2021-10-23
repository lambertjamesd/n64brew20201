Lights1 base_flag_pole_Metal_lights = gdSPDefLights1(
	0x55, 0x61, 0x67,
	0xAB, 0xC3, 0xCF, 0x28, 0x28, 0x28);

Vtx base_flag_pole_Pole_mesh_vtx_cull[8] = {
	{{{-7, 3, -8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-7, 3, 8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-7, 560, 8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-7, 560, -8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{7, 3, -8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{7, 3, 8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{7, 560, 8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{7, 560, -8},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
};

Vtx base_flag_pole_Pole_mesh_vtx_0[24] = {
	{{{0, 3, -8},0, {1008, 496},{0x40, 0x0, 0x92, 0xFF}}},
	{{{0, 560, -8},0, {1008, -16},{0x40, 0x0, 0x92, 0xFF}}},
	{{{7, 560, -4},0, {837, -16},{0x40, 0x0, 0x92, 0xFF}}},
	{{{7, 3, -4},0, {837, 496},{0x40, 0x0, 0x92, 0xFF}}},
	{{{7, 3, -4},0, {837, 496},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{7, 560, -4},0, {837, -16},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{7, 560, 4},0, {667, -16},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{7, 3, 4},0, {667, 496},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{7, 3, 4},0, {667, 496},{0x40, 0x0, 0x6E, 0xFF}}},
	{{{7, 560, 4},0, {667, -16},{0x40, 0x0, 0x6E, 0xFF}}},
	{{{0, 560, 8},0, {496, -16},{0x40, 0x0, 0x6E, 0xFF}}},
	{{{0, 3, 8},0, {496, 496},{0x40, 0x0, 0x6E, 0xFF}}},
	{{{0, 3, 8},0, {496, 496},{0xC0, 0x0, 0x6E, 0xFF}}},
	{{{0, 560, 8},0, {496, -16},{0xC0, 0x0, 0x6E, 0xFF}}},
	{{{-7, 560, 4},0, {325, -16},{0xC0, 0x0, 0x6E, 0xFF}}},
	{{{-7, 3, 4},0, {325, 496},{0xC0, 0x0, 0x6E, 0xFF}}},
	{{{-7, 3, 4},0, {325, 496},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-7, 560, 4},0, {325, -16},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-7, 560, -4},0, {155, -16},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-7, 3, -4},0, {155, 496},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-7, 3, -4},0, {155, 496},{0xC0, 0x0, 0x92, 0xFF}}},
	{{{-7, 560, -4},0, {155, -16},{0xC0, 0x0, 0x92, 0xFF}}},
	{{{0, 560, -8},0, {-16, -16},{0xC0, 0x0, 0x92, 0xFF}}},
	{{{0, 3, -8},0, {-16, 496},{0xC0, 0x0, 0x92, 0xFF}}},
};

Gfx base_flag_pole_Pole_mesh_tri_0[] = {
	gsSPVertex(base_flag_pole_Pole_mesh_vtx_0 + 0, 24, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSP2Triangles(4, 5, 6, 0, 4, 6, 7, 0),
	gsSP2Triangles(8, 9, 10, 0, 8, 10, 11, 0),
	gsSP2Triangles(12, 13, 14, 0, 12, 14, 15, 0),
	gsSP2Triangles(16, 17, 18, 0, 16, 18, 19, 0),
	gsSP2Triangles(20, 21, 22, 0, 20, 22, 23, 0),
	gsSPEndDisplayList(),
};

Gfx mat_base_flag_pole_Metal[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsSPSetLights1(base_flag_pole_Metal_lights),
	gsSPEndDisplayList(),
};

Gfx base_flag_pole_Pole_mesh[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(base_flag_pole_Pole_mesh_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_base_flag_pole_Metal),
	gsSPDisplayList(base_flag_pole_Pole_mesh_tri_0),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

