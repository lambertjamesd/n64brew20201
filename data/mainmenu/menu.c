
#include "menu.h"
#include "models/mars.geo.h"
#include "models/levelplaceholder.geo.h"
#include "images/players_1.h"
#include "images/players_2.h"
#include "images/players_3.h"
#include "images/players_4.h"
#include "images/background.h"
#include "images/title_mission.h"
#include "images/title_lostcontrol.h"
#include "images/title_startbutton.h"
#include "images/arrows.h"
#include "images/title_screen.h"
#include "images/discord_logo.h"

Gfx gMainMenuSolidColor[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_LIGHTING | G_SHADE),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
	gsSPEndDisplayList(),
};

Gfx gMainMenuLevelWireframePass0[] = {
    gsDPPipeSync(),
    gsSPGeometryMode(G_LIGHTING, G_ZBUFFER | G_SHADE),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetEnvColor(64, 255, 32, 255),
    gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT),
    gsSPEndDisplayList(),
};

Gfx gMainMenuLevelWireframePass1[] = {
    gsDPPipeSync(),
    gsDPSetRenderMode(G_RM_AA_ZB_DEC_LINE, G_RM_AA_ZB_DEC_LINE2),
    gsDPSetEnvColor(32, 32, 32, 255),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
    gsSPEndDisplayList(),
};