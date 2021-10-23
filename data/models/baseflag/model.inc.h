Lights1 base_flag_f3d_material_lights = gdSPDefLights1(
	0x0, 0x73, 0x7F,
	0x0, 0xE6, 0xFE, 0x28, 0x28, 0x28);

Vtx base_flag_Flag_mesh_vtx_cull[8] = {
	{{{0, -59, -198},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{0, -59, 0},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{0, 59, 0},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{0, 59, -198},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{31, -59, -198},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{31, -59, 0},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{31, 59, 0},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{31, 59, -198},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
};

Vtx base_flag_Flag_mesh_vtx_0[10] = {
	{{{0, 59, 0},0, {-16, 1008},{0x7E, 0x0, 0x13, 0xFF}}},
	{{{0, -59, 0},0, {-16, 1008},{0x7E, 0x0, 0x13, 0xFF}}},
	{{{31, 31, -198},0, {-16, 1008},{0x7E, 0x0, 0x13, 0xFF}}},
	{{{0, 59, 0},0, {-16, 1008},{0x82, 0x0, 0xED, 0xFF}}},
	{{{31, 31, -198},0, {-16, 1008},{0x82, 0x0, 0xED, 0xFF}}},
	{{{0, -59, 0},0, {-16, 1008},{0x82, 0x0, 0xED, 0xFF}}},
	{{{0, 59, 0},0, {-16, 1008},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{31, 31, -198},0, {-16, 1008},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{31, 31, -198},0, {-16, 1008},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{0, -59, 0},0, {-16, 1008},{0x0, 0x0, 0x7F, 0xFF}}},
};

Gfx base_flag_Flag_mesh_tri_0[] = {
	gsSPVertex(base_flag_Flag_mesh_vtx_0 + 0, 10, 0),
	gsSP2Triangles(0, 1, 2, 0, 3, 4, 5, 0),
	gsSP2Triangles(6, 7, 7, 0, 7, 9, 7, 0),
	gsSP2Triangles(7, 9, 9, 0, 9, 6, 9, 0),
	gsSP2Triangles(9, 6, 6, 0, 6, 7, 6, 0),
	gsSPEndDisplayList(),
};

Gfx mat_base_flag_f3d_material[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, PRIMITIVE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsSPSetLights1(base_flag_f3d_material_lights),
	gsSPEndDisplayList(),
};

Gfx base_flag_Flag_mesh[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(base_flag_Flag_mesh_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_base_flag_f3d_material),
	gsSPDisplayList(base_flag_Flag_mesh_tri_0),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

