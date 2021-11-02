#include "sk64/skelatool_defs.h"

Vtx punchtrail_Trail_normal[] = {
    {{{-88, 51, 0}, 0, {43, 133}, {-98, 58, -57, 255}}},
    {{{-66, 38, 0}, 0, {96, 163}, {-98, 58, -57, 255}}},
    {{{0, -76, 0}, 0, {256, 440}, {-98, 58, -57, 255}}},
    {{{0, -102, 0}, 0, {256, 501}, {-98, 58, -57, 255}}},
    {{{88, 51, 0}, 0, {468, 133}, {0, 58, 114, 255}}},
    {{{66, 38, 0}, 0, {415, 163}, {0, 58, 114, 255}}},
    {{{-66, 38, 0}, 0, {96, 163}, {0, 58, 114, 255}}},
    {{{-88, 51, 0}, 0, {43, 133}, {0, 58, 114, 255}}},
    {{{0, -102, 0}, 0, {256, 501}, {98, 58, -57, 255}}},
    {{{0, -76, 0}, 0, {256, 440}, {98, 58, -57, 255}}},
    {{{66, 38, 0}, 0, {415, 163}, {98, 58, -57, 255}}},
    {{{88, 51, 0}, 0, {468, 133}, {98, 58, -57, 255}}},
};


Vtx punchtrail_Trail_normal_1[] = {
    {{{0, -102, 0}, 0, {768, 501}, {0, -128, 0, 255}}},
    {{{88, 51, 0}, 0, {980, 133}, {0, -128, 0, 255}}},
    {{{-88, 51, 0}, 0, {555, 133}, {0, -128, 0, 255}}},
};


Vtx punchtrail_Trail_normal_2[] = {
    {{{-66, 38, 0}, 0, {96, 163}, {-98, 58, -57, 255}}},
    {{{0, -76, 0}, 0, {256, 440}, {-98, 58, -57, 255}}},
    {{{-44, 25, 0}, 0, {149, 194}, {-98, 58, -57, 255}}},
    {{{0, -51, 0}, 0, {256, 378}, {-98, 58, -57, 255}}},
    {{{66, 38, 0}, 0, {415, 163}, {0, 58, 114, 255}}},
    {{{-66, 38, 0}, 0, {96, 163}, {0, 58, 114, 255}}},
    {{{44, 25, 0}, 0, {362, 194}, {0, 58, 114, 255}}},
    {{{-44, 25, 0}, 0, {149, 194}, {0, 58, 114, 255}}},
    {{{0, -76, 0}, 0, {256, 440}, {98, 58, -57, 255}}},
    {{{66, 38, 0}, 0, {415, 163}, {98, 58, -57, 255}}},
    {{{0, -51, 0}, 0, {256, 378}, {98, 58, -57, 255}}},
    {{{44, 25, 0}, 0, {362, 194}, {98, 58, -57, 255}}},
};


Vtx punchtrail_Trail_normal_3[] = {
    {{{0, -25, 0}, 0, {256, 317}, {98, 58, -57, 255}}},
    {{{22, 12, 0}, 0, {309, 225}, {98, 58, -57, 255}}},
    {{{22, 12, 0}, 0, {309, 225}, {0, 58, 114, 255}}},
    {{{-22, 12, 0}, 0, {202, 225}, {0, 58, 114, 255}}},
    {{{-22, 12, 0}, 0, {202, 225}, {-98, 58, -57, 255}}},
    {{{0, -25, 0}, 0, {256, 317}, {-98, 58, -57, 255}}},
    {{{-44, 25, 0}, 0, {149, 194}, {-98, 58, -57, 255}}},
    {{{0, -51, 0}, 0, {256, 378}, {-98, 58, -57, 255}}},
    {{{44, 25, 0}, 0, {362, 194}, {0, 58, 114, 255}}},
    {{{-44, 25, 0}, 0, {149, 194}, {0, 58, 114, 255}}},
    {{{0, -51, 0}, 0, {256, 378}, {98, 58, -57, 255}}},
    {{{44, 25, 0}, 0, {362, 194}, {98, 58, -57, 255}}},
};


Vtx punchtrail_Trail_normal_4[] = {
    {{{0, -25, 0}, 0, {256, 317}, {98, 58, -57, 255}}},
    {{{0, 0, 0}, 0, {256, 256}, {98, 58, -57, 255}}},
    {{{22, 12, 0}, 0, {309, 225}, {98, 58, -57, 255}}},
    {{{22, 12, 0}, 0, {309, 225}, {0, 58, 114, 255}}},
    {{{0, 0, 0}, 0, {256, 256}, {0, 58, 114, 255}}},
    {{{-22, 12, 0}, 0, {202, 225}, {0, 58, 114, 255}}},
    {{{-22, 12, 0}, 0, {202, 225}, {-98, 58, -57, 255}}},
    {{{0, 0, 0}, 0, {256, 256}, {-98, 58, -57, 255}}},
    {{{0, -25, 0}, 0, {256, 317}, {-98, 58, -57, 255}}},
};


Gfx punchtrail_model_gfx[] = {
	gsDPPipeSync(),
	gsSPClearGeometryMode(G_LIGHTING | G_CULL_BOTH),
    gsDPSetPrimColor(255, 255, 200, 200, 200, 128),
	gsDPSetCombineLERP(0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0, 0, 0, 0, PRIMITIVE, PRIMITIVE, 0, SHADE, 0),
    // Bone.001
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 1, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal[1], 2, 0),
    gsSPVertex(&punchtrail_Trail_normal[5], 2, 2),
    gsSPVertex(&punchtrail_Trail_normal[9], 2, 4),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 0, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal[0], 1, 6),
    gsSPVertex(&punchtrail_Trail_normal[3], 2, 7),
    gsSPVertex(&punchtrail_Trail_normal[7], 2, 9),
    gsSPVertex(&punchtrail_Trail_normal[11], 1, 11),
    gsSP2Triangles(6, 0, 1, 0, 6, 1, 7, 0),
    gsSP2Triangles(8, 2, 3, 0, 8, 3, 9, 0),
    gsSP2Triangles(10, 4, 5, 0, 10, 5, 11, 0),
    gsSPVertex(&punchtrail_Trail_normal_1[0], 3, 0),
    gsSP1Triangle(0, 1, 2, 0),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone.002
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 2, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal_2[2], 2, 0),
    gsSPVertex(&punchtrail_Trail_normal_2[6], 2, 2),
    gsSPVertex(&punchtrail_Trail_normal_2[10], 2, 4),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone.001
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 1, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal_2[0], 2, 6),
    gsSPVertex(&punchtrail_Trail_normal_2[4], 2, 8),
    gsSPVertex(&punchtrail_Trail_normal_2[8], 2, 10),
    gsSP2Triangles(6, 0, 1, 0, 6, 1, 7, 0),
    gsSP2Triangles(8, 2, 3, 0, 8, 3, 9, 0),
    gsSP2Triangles(10, 4, 5, 0, 10, 5, 11, 0),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone.003
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 3, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal_3[0], 6, 0),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone.002
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 2, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal_3[6], 6, 6),
    gsSP2Triangles(6, 4, 5, 0, 6, 5, 7, 0),
    gsSP2Triangles(8, 2, 3, 0, 8, 3, 9, 0),
    gsSP2Triangles(10, 0, 1, 0, 10, 1, 11, 0),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone.004
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 4, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal_4[1], 1, 0),
    gsSPVertex(&punchtrail_Trail_normal_4[4], 1, 1),
    gsSPVertex(&punchtrail_Trail_normal_4[7], 1, 2),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    // Bone.003
    gsSPMatrix((Mtx*)MATRIX_TRANSFORM_SEGMENT_ADDRESS + 3, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH),
    gsSPVertex(&punchtrail_Trail_normal_4[0], 1, 3),
    gsSPVertex(&punchtrail_Trail_normal_4[2], 2, 4),
    gsSPVertex(&punchtrail_Trail_normal_4[5], 2, 6),
    gsSPVertex(&punchtrail_Trail_normal_4[8], 1, 8),
    gsSP2Triangles(3, 0, 4, 0, 5, 1, 6, 0),
    gsSP1Triangle(7, 2, 8, 0),
    gsSPPopMatrix(G_MTX_MODELVIEW),
    gsSPEndDisplayList(),
};
