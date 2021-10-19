
#include <ultra64.h>
#include "sk64/skelatool_defs.h"

#include "example/geometry_geo.inc.h"
#include "dogminion/model.inc.h"
#include "base/model.inc.h"
#include "doglow/model.inc.h"


Vtx mario_Cube_mesh_vtx_cull[8] = {
	{{{-213, -213, -213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{-213, -213, 213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{-213, 213, 213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{-213, 213, -213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{213, -213, -213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{213, -213, 213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{213, 213, 213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
	{{{213, 213, -213},0, {-16, -16},{0x0, 0x0, 0x0, 0xFF}}},
};

Vtx mario_Cube_mesh_vtx_0[24] = {
	{{{213, 213, -213},0, {1008, -16},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{-213, 213, -213},0, {-16, -16},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{-213, 213, 213},0, {-16, 1008},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{213, 213, 213},0, {1008, 1008},{0x0, 0x7F, 0x0, 0xFF}}},
	{{{213, -213, 213},0, {1008, 1008},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{213, 213, 213},0, {1008, -16},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{-213, 213, 213},0, {-16, -16},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{-213, -213, 213},0, {-16, 1008},{0x0, 0x0, 0x7F, 0xFF}}},
	{{{-213, -213, 213},0, {-16, 1008},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-213, 213, 213},0, {-16, -16},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-213, 213, -213},0, {1008, -16},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-213, -213, -213},0, {1008, 1008},{0x81, 0x0, 0x0, 0xFF}}},
	{{{-213, -213, -213},0, {-16, -16},{0x0, 0x81, 0x0, 0xFF}}},
	{{{213, -213, -213},0, {1008, -16},{0x0, 0x81, 0x0, 0xFF}}},
	{{{213, -213, 213},0, {1008, 1008},{0x0, 0x81, 0x0, 0xFF}}},
	{{{-213, -213, 213},0, {-16, 1008},{0x0, 0x81, 0x0, 0xFF}}},
	{{{213, -213, -213},0, {1008, 1008},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{213, 213, -213},0, {1008, -16},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{213, 213, 213},0, {-16, -16},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{213, -213, 213},0, {-16, 1008},{0x7F, 0x0, 0x0, 0xFF}}},
	{{{-213, -213, -213},0, {-16, 1008},{0x0, 0x0, 0x81, 0xFF}}},
	{{{-213, 213, -213},0, {-16, -16},{0x0, 0x0, 0x81, 0xFF}}},
	{{{213, 213, -213},0, {1008, -16},{0x0, 0x0, 0x81, 0xFF}}},
	{{{213, -213, -213},0, {1008, 1008},{0x0, 0x0, 0x81, 0xFF}}},
};

Gfx mario_Cube_mesh_tri_0[] = {
	gsSPVertex(mario_Cube_mesh_vtx_0 + 0, 24, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSP2Triangles(4, 5, 6, 0, 4, 6, 7, 0),
	gsSP2Triangles(8, 9, 10, 0, 8, 10, 11, 0),
	gsSP2Triangles(12, 13, 14, 0, 12, 14, 15, 0),
	gsSP2Triangles(16, 17, 18, 0, 16, 18, 19, 0),
	gsSP2Triangles(20, 21, 22, 0, 20, 22, 23, 0),
	gsSPEndDisplayList(),
};


Gfx mat_mario_sm64_material[] = {
	gsDPPipeSync(),
	gsSPClearGeometryMode(G_LIGHTING),
	gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, PRIMITIVE, 0, 0, 0, ENVIRONMENT),
	gsDPSetPrimColor(0, 0, 187, 0, 142, 255),
	gsSPEndDisplayList(),
};


Gfx mario_Cube_mesh[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	// gsSPVertex(mario_Cube_mesh_vtx_cull + 0, 8, 0),
	// gsSPSetGeometryMode(G_LIGHTING),
	// gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_mario_sm64_material),
	gsSPDisplayList(mario_Cube_mesh_tri_0),
	gsDPPipeSync(),
	// gsSPSetGeometryMode(G_LIGHTING),
	// gsSPClearGeometryMode(G_TEXTURE_GEN),
	// gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
	// gsSPTexture(65535, 65535, 0, 0, 0),
	gsSPEndDisplayList(),
};
