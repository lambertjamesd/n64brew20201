/*====================================================================
 * gfx.c
 *
 * Synopsis:
 *
 * This code implements the application graphics stuff
 * 
 * 
 * 
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/

/*---------------------------------------------------------------------*
        Copyright (C) 1998 Nintendo. (Originated by SGI)
        
        $RCSfile: gfx.c,v $
        $Revision: 1.1.1.1 $
        $Date: 2002/05/02 03:27:21 $
 *---------------------------------------------------------------------*/

#include <ultralog.h>
#include <assert.h>

#include "gfx.h"
#include "moba64.h"
#include "gfxvalidator/validator.h"
#include "gfxvalidator/error_printer.h"
#include "util/memory.h"
#include "../data/models/example/geometry.h"
#include "sk64/skelatool_object.h"
#include "sk64/skelatool_defs.h"

#include "../data/levels/test/test.h"
#include "scene/scene_management.h"

/*
 * graphics globals
 */

extern OSSched         sc;
extern OSMesgQueue     *sched_cmdQ;
extern GFXInfo         gInfo[];

extern s8    logoPos_x;
extern s8    logoPos_y;
extern s8    logoPos_z;
extern f32   logoScale_x;
extern f32   logoScale_y;
extern f32   logoScale_z;
extern f32   logoVeloc;

u32	ucode_index = 0;

static u32          framecount;

static char         *staticSegment = 0;

static char         *characterSegment = 0;

static char         *levelSegment = 0;

static struct SkelatoolObject objectTest;

#define RDP_OUTPUT_SIZE 0x4000
static void* rdp_output;

extern char _charactersSegmentRomStart[];
extern char _charactersSegmentRomEnd[];

extern char _level_testSegmentRomStart[];
extern char _level_testSegmentRomEnd[];

extern char _character_animationsSegmentRomStart[];

void doLogo(struct RenderState *dynamicp);


void initGFX() 
{    
    u32 len = (u32)(_staticSegmentRomEnd - _staticSegmentRomStart);

    assert (len < GFX_DL_BUF_SIZE * sizeof(Gfx));
    staticSegment = malloc(len);
    romCopy(_staticSegmentRomStart, staticSegment, len);

    LOAD_SEGMENT(characters, characterSegment);
    LOAD_SEGMENT(level_test, levelSegment);

    skInitObject(
        &objectTest, 
        output_model_gfx, 
        OUTPUT_DEFAULT_BONES_COUNT, 
        0
    );

    romCopy(
        ANIM_DATA_ROM_ADDRESS(_character_animationsSegmentRomStart, output_default_bones), 
        (void*)objectTest.boneTransforms, 
        sizeof(struct Transform) * objectTest.numberOfBones
    );

    loadLevelScene();
    gCurrentLevel.levelDL = test_level_geometry;
    gCurrentLevel.cameras[0].transform.position.z = 1000.0f;

    quatIdent(&objectTest.boneTransforms[0].rotation);
    
    gInfo[0].msg.gen.type = OS_SC_DONE_MSG;
    gInfo[1].msg.gen.type = OS_SC_DONE_MSG;
    /* The Vi manager was started by scheduler by this point in time */
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
}

void* initGFXBuffers(void* maxMemory) {
    u16* currBuffer = maxMemory;
    currBuffer -= SCREEN_HT * SCREEN_WD;
    gInfo[0].cfb = currBuffer;
    currBuffer -= SCREEN_HT * SCREEN_WD;
    gInfo[1].cfb = currBuffer;

    currBuffer -= RDP_OUTPUT_SIZE / sizeof(u16);
    rdp_output = currBuffer;

    return currBuffer;
}

void createGfxTask(GFXInfo *i) 
{
    static int firsttime = 1;
    struct RenderState *renderState;
    OSScTask *t;
    
    /**** pointers to build the display list. ****/
    renderState = &i->renderState;
    renderStateInit(renderState);

    /**** Tell RCP where each segment is  ****/
    gSPSegment(renderState->dl++, 0, 0);	/* physical addressing segment */
    gSPSegment(renderState->dl++, STATIC_SEGMENT,  osVirtualToPhysical(staticSegment));
    gSPSegment(renderState->dl++, CHARACTER_SEGMENT, osVirtualToPhysical(characterSegment));
    gSPSegment(renderState->dl++, LEVEL_SEGMENT, osVirtualToPhysical(levelSegment));

    /**** Graphics pipeline state initialization ****/
    gSPDisplayList(renderState->dl++, setup_rspstate);
    if (firsttime) {
        gSPDisplayList(renderState->dl++, rdpstateinit_dl);
	firsttime = 0;
    }

    gSPDisplayList(renderState->dl++, setup_rdpstate);
    
    /**** clear zbuffer, z = max z, dz = 0  ****/
    gDPSetDepthImage(renderState->dl++, osVirtualToPhysical(zbuffer));
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_FILL);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		     osVirtualToPhysical(zbuffer));
    gDPSetFillColor(renderState->dl++, (GPACK_ZDZ(G_MAXFBZ,0) << 16 |
			       GPACK_ZDZ(G_MAXFBZ,0)));
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	
    /**** Clear framebuffer cvg = FULL or 1  ****/
    gDPPipeSync(renderState->dl++);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		     osVirtualToPhysical(i->cfb));
    /* clearcolor is 32-bits (2 pixels wide) because fill mode
     * does two pixels at a time.
     */
    gDPSetFillColor(renderState->dl++, (GPACK_RGBA5551(0, 0, 0, 1) << 16 | 
			       GPACK_RGBA5551(0, 0, 0, 1)));
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE); 

    /**** Draw objects */
    doLogo(renderState);

    
    /**** Put an end on the top-level display list  ****/
    gDPFullSync(renderState->dl++);
    gSPEndDisplayList(renderState->dl++);

    /* Flush the dynamic segment */
    renderStateFlushCache(renderState);

    /* build graphics task */

    t = &i->task;
    t->list.t.data_ptr = (u64 *) renderState->glist;
    t->list.t.data_size = (s32)(renderState->dl - renderState->glist) * sizeof (Gfx);
    t->list.t.type = M_GFXTASK;
    t->list.t.flags = OS_TASK_LOADABLE;
    // t->list.t.flags = 0;
    t->list.t.ucode_boot = (u64 *)rspbootTextStart;
    t->list.t.ucode_boot_size = ((s32) rspbootTextEnd - (s32) rspbootTextStart);
    t->list.t.ucode =      (u64 *) gspF3DEX2_fifoTextStart;
    t->list.t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart; 
    t->list.t.output_buff = (u64 *) rdp_output;
    t->list.t.output_buff_size = ((u64 *) rdp_output + RDP_OUTPUT_SIZE/sizeof(u64) - 2);
    t->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
    // t->list.t.ucode_size = 0;
    t->list.t.dram_stack = (u64 *) dram_stack;
    t->list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;
    t->list.t.yield_data_ptr = (u64 *) gfxYieldBuf;
    t->list.t.yield_data_size = OS_YIELD_DATA_SIZE;

    t->next     = 0;                   /* paranoia */
    // t->state = 0;
    t->flags	= (OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP | OS_SC_LAST_TASK |
		   OS_SC_SWAPBUFFER);
    t->msgQ     = &gfxFrameMsgQ;       /* reply to when finished */
    t->msg      = (OSMesg)&i->msg;     /* reply with this message */
    t->framebuffer = (void *)i->cfb;

    // struct GFXValidationResult validationResult;
    // gfxValidate(&t->list, 0, &validationResult);

    // if (validationResult.reason != GFXValidatorErrorNone) {
    //     struct GFXStringPrinter stringPrinter;
    //     gfxInitStringPrinter(&stringPrinter, malloc(256), 256);
    //     gfxGenerateReadableMessage(&validationResult, gfxStringPrinter, &stringPrinter);
    //     free(stringPrinter.output);
    // }

    osSendMesg(sched_cmdQ, (OSMesg) t, OS_MESG_BLOCK); 
    
    framecount++;
}

extern Gfx mario_Cube_mesh[];
extern Gfx mat_mario_sm64_material[];
extern Gfx output_model_gfx[];
extern Gfx mario_Cube_mesh_tri_0[];
extern Gfx mario_Cube_empty[];

/*
 * Draw the SGI Logo
 *
 * You can mix the gu and the gSP commands.  The important item to 
 * remember is that the cache must be flushed of any dynamic data
 * before the RSP starts reading the command list.
 */
void doLogo(struct RenderState *renderState)
{
    u16		   perspNorm;
    static float   logo_theta = 0;
    /*
     * You must make the call to gSPPerspNormalize() in addition to 
     * using the perspective projection matrix.
     */
    Mtx* projection = renderStateRequestMatrices(renderState, 1);
    guPerspective(projection, &perspNorm,
		  33, 320.0/240.0, 400, 2000, 1.0);
    gSPPerspNormalize(renderState->dl++, perspNorm);

    Mtx* viewing = renderStateRequestMatrices(renderState, 1);
    guLookAt(viewing, 
	     0, 0, 1000,
	     0, 0, 0,
	     0, 1, 0);
    

    /* draw the background first */
    Mtx* bg_model = renderStateRequestMatrices(renderState, 1);
    guScale(bg_model, 12.0, 7.0, 1.0);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(projection), 
	       G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(viewing), 
	       G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(bg_model), 
	       G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPDisplayList(renderState->dl++, bg_dl);

    levelSceneRender(&gCurrentLevel, renderState);

    // float scale = 0.5f;

    // /* Position the logo: */
    // Mtx* logo_trans = renderStateRequestMatrices(renderState, 1);
    // guTranslate(logo_trans, logoPos_x*4, logoPos_y*2, logoPos_z);
    // /* Scale the logo */
    // Mtx* logo_scale = renderStateRequestMatrices(renderState, 1);
    // guScale(logo_scale, logoScale_x * scale, logoScale_y * scale, logoScale_z * scale);
    // /* Rotate the logo */
    // Mtx* logo_rotate = renderStateRequestMatrices(renderState, 1);
    // guRotate(logo_rotate, logo_theta, 1.0, 1.0, 1.0);

    // /* Setup model matrix */
    // gSPMatrix(renderState->dl++, osVirtualToPhysical(projection), 
	//        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    // gSPMatrix(renderState->dl++, osVirtualToPhysical(viewing), 
	//        G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    // gSPMatrix(renderState->dl++, osVirtualToPhysical(logo_trans), 
	//       G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    // gSPMatrix(renderState->dl++, osVirtualToPhysical(logo_scale), 
	//       G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    // gSPMatrix(renderState->dl++, osVirtualToPhysical(logo_rotate), 
	//       G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    /* Draw the logo */
    // gSPDisplayList(renderState->dl++, logo_dl);
    // gSPDisplayList(renderState->dl++, test_level_geometry);
    // gSPDisplayList(renderState->dl++, mat_mario_sm64_material);
    // gSPDisplayList(renderState->dl++, mario_Cube_mesh);

    // objectTest.boneTransforms[1].position.y = 4 * 256.0f + sinf(logo_theta / 180.0f) * 32.0f;
    // quatAxisAngle(&gUp, logo_theta / 170.0f, &objectTest.boneTransforms[1].rotation);
    
    // skRenderObject(&objectTest, renderState);

    /* calculate theta for next frame */
    logo_theta += logoVeloc;
}
