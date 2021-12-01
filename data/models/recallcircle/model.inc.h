
Vtx RecallCircle_Cylinder_mesh_vtx_cull[8] = {
	{{{-100, 0, -100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-100, 0, 100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-100, 40, 100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{-100, 40, -100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{100, 0, -100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{100, 0, 100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{100, 40, 100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
	{{{100, 40, -100},0, {-16, -16},{0x0, 0x0, 0x0, 0x0}}},
};

Vtx RecallCircle_Cylinder_mesh_vtx_0[18] = {
	{{{0, 0, -100},0, {1008, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{0, 40, -100},0, {1008, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{71, 40, -71},0, {880, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{71, 0, -71},0, {880, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{100, 40, 0},0, {752, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{100, 0, 0},0, {752, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{71, 40, 71},0, {624, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{71, 0, 71},0, {624, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{0, 40, 100},0, {496, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{0, 0, 100},0, {496, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{-71, 40, 71},0, {368, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{-71, 0, 71},0, {368, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{-100, 40, 0},0, {240, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{-100, 0, 0},0, {240, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{-71, 40, -71},0, {112, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{-71, 0, -71},0, {112, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
	{{{0, 40, -100},0, {-16, -16},{0xFF, 0xFF, 0xFF, 0x00}}},
	{{{0, 0, -100},0, {-16, 496},{0xFF, 0xFF, 0xFF, 0xFF}}},
};

Gfx RecallCircle_Cylinder_mesh_tri_0[] = {
	gsSPVertex(RecallCircle_Cylinder_mesh_vtx_0 + 0, 18, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSP2Triangles(3, 2, 4, 0, 3, 4, 5, 0),
	gsSP2Triangles(5, 4, 6, 0, 5, 6, 7, 0),
	gsSP2Triangles(7, 6, 8, 0, 7, 8, 9, 0),
	gsSP2Triangles(9, 8, 10, 0, 9, 10, 11, 0),
	gsSP2Triangles(11, 10, 12, 0, 11, 12, 13, 0),
	gsSP2Triangles(13, 12, 14, 0, 13, 14, 15, 0),
	gsSP2Triangles(15, 14, 16, 0, 15, 16, 17, 0),
	gsSPEndDisplayList(),
};

Gfx RecallCircle_Cylinder_mesh[] = {
	gsDPPipeSync(),
	gsSPClearGeometryMode(G_LIGHTING | G_CULL_BOTH),
	gsSPVertex(RecallCircle_Cylinder_mesh_vtx_cull + 0, 8, 0),
	gsSPCullDisplayList(0, 7),
	gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0),
	gsSPDisplayList(RecallCircle_Cylinder_mesh_tri_0),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_CULL_BACK),
	gsSPEndDisplayList(),
};

