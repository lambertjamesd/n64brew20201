
Gfx gUseSolidColor[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetEnvColor(255, 255, 255, 255),
    gsDPSetCombineLERP(0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT),
	gsSPEndDisplayList(),
};