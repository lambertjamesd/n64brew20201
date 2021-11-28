#ifndef _TEAM_DATA_H
#define _TEAM_DATA_H

#include <ultra64.h>
#include "math/color.h"

// Blue Team
#define TEAM_MAIN_COLOR_0   58, 222, 250
#define TEAM_MAIN_COLOR_SATURATED_0   24, 152, 255
#define TEAM_DARK_COLOR_0   45, 49, 66
// Detail- 162, 62, 72

// Pink Team
#define TEAM_MAIN_COLOR_1   247, 170, 248
#define TEAM_MAIN_COLOR_SATURATED_1   255, 120, 180
#define TEAM_DARK_COLOR_1   102, 86, 135
// Detail- 106, 141, 115

// Yellow Team
#define TEAM_MAIN_COLOR_2   255, 242, 80
#define TEAM_MAIN_COLOR_SATURATED_2   255, 200, 20
#define TEAM_DARK_COLOR_2   43, 54, 69
// Detail- 189, 78, 31

// Green Team
#define TEAM_MAIN_COLOR_3   0, 239, 107
#define TEAM_MAIN_COLOR_SATURATED_3   0, 255, 50
#define TEAM_DARK_COLOR_3   43, 54, 69
// Detail- 189, 78, 31

#define TEAM_DAMAGE_COLOR   202, 43, 62

extern struct Coloru8 gTeamColors[];
extern struct Coloru8 gTeamColorsSaturated[];
extern struct Coloru8 gTeamDarkColors[];
extern Gfx gTeamTexture[];

#define gDPUseTeamPallete(pkt, teamNumber, textureShift) \
    gDPSetTile(pkt, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0, 0, teamNumber, G_TX_WRAP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 4, 0); \
	gSPTexture(pkt, 65535 >> textureShift, 65535 >> textureShift, 0, 0, 1); \
	gDPSetTextureLUT(pkt, G_TT_RGBA16)

#define NUETRAL_FACTION_INDEX   4
#define DAMAGE_PALLETE_INDEX    5

#endif