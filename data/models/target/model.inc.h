u64 ItemPickup_Target_i4[] = {
	0x0000000000159cef, 0xfec9510000000000, 0x0000000029efffff, 0xfffffe9200000000, 0x00000018ffffffff, 0xffffffff80000000, 0x000002cfffffffff, 0xfffffffffc100000, 
	0x00003effffffffff, 0xffffffffffd30000, 0x0002effffffe95ff, 0xff59effffffd1000, 0x000cfffffe8100ff, 0xff0018efffffc100, 0x008fffffd30000ff, 0xff00003dfffff800, 
	0x02fffffd200000ff, 0xff000002dffffe10, 0x09ffffe3000000ff, 0xff0000003fffff90, 0x1effff80000000ff, 0xff00000008ffffe1, 0x5ffffe10000000ff, 0xff00000001effff5, 
	0x9ffff900000000ff, 0xff000000009ffff9, 0xcffff500000000ff, 0xff000000005ffffc, 0xefffffffffffffff, 0xfffffffffffffffe, 0xffffffffffffffff, 0xffffffffffffffff, 
	0xffffffffffffffff, 0xffffffffffffffff, 0xefffffffffffffff, 0xfffffffffffffffe, 0xcffff500000000ff, 0xff000000005ffffc, 0x9ffff900000000ff, 0xff000000009ffff9, 
	0x5ffffe10000000ff, 0xff00000001effff5, 0x1effff80000000ff, 0xff00000008ffffe1, 0x09fffff3000000ff, 0xff0000003fffff90, 0x02fffffd200000ff, 0xff000002dfffff20, 
	0x008fffffd30000ff, 0xff00003dfffff800, 0x000cffffff8100ff, 0xff0018ffffffc000, 0x0001dffffffe95ff, 0xff59effffffd2000, 0x00003dffffffffff, 0xffffffffffd30000, 
	0x000001bfffffffff, 0xfffffffffc100000, 0x00000008efffffff, 0xffffffff80000000, 0x0000000019efffff, 0xfffffe9200000000, 0x0000000000159cef, 0xfec9510000000000, 
	
};

Vtx ItemPickup_Target_mesh_vtx_0[4] = {
	{{{-138, 0, 138},0, {-16, 1008},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{138, 0, 138},0, {1008, 1008},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{138, 0, -138},0, {1008, -16},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{-138, 0, -138},0, {-16, -16},{0x0, 0x7F, 0x0, 0xFF}}},
};

Gfx ItemPickup_Target_mesh_tri_0[] = {
	gsSPVertex(ItemPickup_Target_mesh_vtx_0 + 0, 4, 0),
	gsSP1Triangle(0, 1, 2, 0),
	gsSP1Triangle(0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_ItemPickup_Target[] = {
	gsDPPipeSync(),
	gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, 0, 0, 0, TEXEL0, 0, 0, 0, PRIMITIVE, 0, 0, 0, TEXEL0),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPTileSync(),
	gsDPSetTextureLUT(G_TT_NONE),
	gsDPSetTextureImage(G_IM_FMT_I, G_IM_SIZ_16b, 1, ItemPickup_Target_i4),
	gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 255, 1024),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_I, G_IM_SIZ_4b, 2, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, 0),
	gsDPSetTileSize(0, 0, 0, 124, 124),
	gsDPSetPrimColor(0, 0, 254, 0, 15, 255),
	gsSPEndDisplayList(),
};

Gfx ItemPickup_Target_mesh[] = {
	gsSPDisplayList(mat_ItemPickup_Target),
	gsSPDisplayList(ItemPickup_Target_mesh_tri_0),
	gsDPPipeSync(),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPClearGeometryMode(G_TEXTURE_GEN),
	gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};

