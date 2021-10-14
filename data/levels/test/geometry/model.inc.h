Lights1 Test_f3d_Rocks_lights = gdSPDefLights1(
	0x44, 0xA, 0x2,
	0x88, 0x15, 0x5, 0x28, 0x28, 0x28);

Vtx test_level_geometry_vtx_0[29] = {
	{{{-69, -47, -96},0, {131, 496},{0xFD, 0x81, 0x0, 0xFF}}},
	{{{106, -49, -80},0, {370, 489},{0xFF, 0x81, 0xFF, 0xFF}}},
	{{{89, -51, 94},0, {368, 242},{0xFE, 0x81, 0x0, 0xFF}}},
	{{{89, -51, 94},0, {370, 240},{0xFE, 0x81, 0x0, 0xFF}}},
	{{{-82, -45, 68},0, {112, 248},{0xFC, 0x81, 0x0, 0xFF}}},
	{{{7, 89, -55},0, {623, 624},{0x3, 0x24, 0x86, 0xFF}}},
	{{{68, 72, -52},0, {624, 496},{0x10, 0x21, 0x86, 0xFF}}},
	{{{106, -49, -80},0, {370, 489},{0xC, 0x1F, 0x85, 0xFF}}},
	{{{-69, -47, -96},0, {368, 733},{0x4, 0x22, 0x86, 0xFF}}},
	{{{-55, 78, -51},0, {622, 753},{0xF1, 0x2C, 0x8A, 0xFF}}},
	{{{89, -51, 94},0, {368, 242},{0xFB, 0x28, 0x78, 0xFF}}},
	{{{-45, 52, 48},0, {624, -16},{0xF2, 0x23, 0x79, 0xFF}}},
	{{{-82, -45, 68},0, {370, -16},{0xEF, 0x1F, 0x7A, 0xFF}}},
	{{{28, 67, 50},0, {624, 112},{0xFF, 0x2C, 0x77, 0xFF}}},
	{{{89, 57, 46},0, {618, 248},{0xF, 0x33, 0x73, 0xFF}}},
	{{{106, -49, -80},0, {370, 489},{0x7D, 0x17, 0xFD, 0xFF}}},
	{{{89, 57, 46},0, {618, 248},{0x7D, 0x15, 0x0, 0xFF}}},
	{{{89, -51, 94},0, {368, 242},{0x7E, 0x5, 0xC, 0xFF}}},
	{{{68, 72, -52},0, {624, 496},{0x76, 0x2A, 0xED, 0xFF}}},
	{{{-82, -45, 68},0, {370, 1000},{0x85, 0x1F, 0x3, 0xFF}}},
	{{{-55, 78, -51},0, {622, 753},{0x84, 0x1D, 0x1, 0xFF}}},
	{{{-69, -47, -96},0, {368, 733},{0x83, 0x12, 0xF7, 0xFF}}},
	{{{-45, 52, 48},0, {624, 1008},{0x8C, 0x2F, 0x16, 0xFF}}},
	{{{68, 72, -52},0, {624, 496},{0x1D, 0x7A, 0x12, 0xFF}}},
	{{{28, 67, 50},0, {752, 249},{0x3, 0x7D, 0x18, 0xFF}}},
	{{{89, 57, 46},0, {618, 248},{0x14, 0x7D, 0xF, 0xFF}}},
	{{{7, 89, -55},0, {752, 496},{0x2, 0x7C, 0x1A, 0xFF}}},
	{{{-55, 78, -51},0, {880, 495},{0xEA, 0x79, 0x1F, 0xFF}}},
	{{{-45, 52, 48},0, {880, 240},{0xE8, 0x78, 0x21, 0xFF}}},
};

Gfx test_level_geometry_tri_0[] = {
	gsSPVertex(test_level_geometry_vtx_0 + 0, 15, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSP1Triangle(0, 3, 4, 0),
	gsSP1Triangle(5, 6, 7, 0),
	gsSP1Triangle(8, 5, 7, 0),
	gsSP1Triangle(9, 5, 8, 0),
	gsSP1Triangle(10, 11, 12, 0),
	gsSP1Triangle(10, 13, 11, 0),
	gsSP1Triangle(14, 13, 10, 0),
	gsSPVertex(test_level_geometry_vtx_0 + 15, 14, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSP1Triangle(3, 1, 0, 0),
	gsSP1Triangle(4, 5, 6, 0),
	gsSP1Triangle(7, 5, 4, 0),
	gsSP1Triangle(8, 9, 10, 0),
	gsSP1Triangle(8, 11, 9, 0),
	gsSP1Triangle(11, 12, 9, 0),
	gsSP1Triangle(9, 12, 13, 0),
	gsSPEndDisplayList(),
};

Gfx mat_Test_f3d_Rocks[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsSPSetLights1(Test_f3d_Rocks_lights),
	gsSPEndDisplayList(),
};

Gfx test_level_geometry[] = {
	gsSPDisplayList(mat_Test_f3d_Rocks),
	gsSPDisplayList(test_level_geometry_tri_0),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

