#include "team_data.h"

struct Coloru8 gTeamColors[] = {
    {TEAM_MAIN_COLOR_0, 255},
    {TEAM_MAIN_COLOR_1, 255},
    {TEAM_MAIN_COLOR_2, 255},
    {TEAM_MAIN_COLOR_3, 255},
    {200, 200, 200, 255}, // neutral faction
};

struct Coloru8 gTeamDarkColors[] = {
    {TEAM_DARK_COLOR_0, 255},
    {TEAM_DARK_COLOR_1, 255},
    {TEAM_DARK_COLOR_2, 255},
    {TEAM_DARK_COLOR_3, 255},
    {100, 100, 100, 255}, // neutral faction
};

u64 gTeamColorsTexture[] = {
	0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 0x0000000011111111, 
	0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 0x2222222233333333, 
	
};

u64 gTeamColorsBluePallete[] = {
	0x3efd2991a21395ef, 
};

u64 gTeamColorsPinkPallete[] = {
	0xf57d62a16c5dd5f5, 
};

u64 gTeamColorsYellowPallete[] = {
	0xff552991ba49dee9, 
};

u64 gTeamColorsGreenPallete[] = {
	0x075b10c9a533b6f1, 
};

Gfx gTeamPalleteTexture0[] = {
	gsDPPipeSync(),
	gsSPTexture(65535 >> 1, 65535 >> 1, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, gTeamColorsBluePallete),
    gsDPSetTextureFilter(G_TF_AVERAGE),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 3),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_16b, 1, gTeamColorsTexture),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 63, 2048),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPSetTileSize(0, 0, 0, 60, 60),
	gsSPEndDisplayList(),
};

Gfx gTeamPalleteTexture1[] = {
	gsDPPipeSync(),
	gsSPTexture(65535 >> 1, 65535 >> 1, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, gTeamColorsPinkPallete),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 3),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_16b, 1, gTeamColorsTexture),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 63, 2048),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPSetTileSize(0, 0, 0, 60, 60),
	gsSPEndDisplayList(),
};

Gfx gTeamPalleteTexture2[] = {
	gsDPPipeSync(),
	gsSPTexture(65535 >> 1, 65535 >> 1, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, gTeamColorsYellowPallete),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 3),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_16b, 1, gTeamColorsTexture),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 63, 2048),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPSetTileSize(0, 0, 0, 60, 60),
	gsSPEndDisplayList(),
};

Gfx gTeamPalleteTexture3[] = {
	gsDPPipeSync(),
	gsSPTexture(65535 >> 1, 65535 >> 1, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_RGBA16),
	gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, gTeamColorsGreenPallete),
	gsDPTileSync(),
	gsDPSetTile(0, 0, 0, 256, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadSync(),
	gsDPLoadTLUTCmd(7, 3),
	gsDPPipeSync(),
	gsDPTileSync(),
	gsDPSetTextureImage(G_IM_FMT_CI, G_IM_SIZ_16b, 1, gTeamColorsTexture),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPLoadSync(),
	gsDPLoadBlock(7, 0, 0, 63, 2048),
	gsDPPipeSync(),
	gsDPSetTile(G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0),
	gsDPSetTileSize(0, 0, 0, 60, 60),
	gsSPEndDisplayList(),
};

Gfx* gTeamPalleteTexture[] = {
    gTeamPalleteTexture0,
    gTeamPalleteTexture1,
    gTeamPalleteTexture2,
    gTeamPalleteTexture3,
};