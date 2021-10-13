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

static struct SkelatoolObject objectTest;

extern char _charactersSegmentRomStart[];
extern char _charactersSegmentRomEnd[];

extern char _character_animationsSegmentRomStart[];

void doLogo(Dynamic *dynamicp);


void* initGFX(void* maxMemory) 
{    
    u32 len = (u32)(_staticSegmentRomEnd - _staticSegmentRomStart);

    assert (len < GFX_DL_BUF_SIZE * sizeof(Gfx));
    staticSegment = malloc(len);
    romCopy(_staticSegmentRomStart, staticSegment, len);

    len = (u32)(_charactersSegmentRomEnd - _charactersSegmentRomStart);
    characterSegment = malloc(len);
    romCopy(_charactersSegmentRomStart, characterSegment, len);

    skInitObject(
        &objectTest, 
        output_model_gfx, 
        // OUTPUT_DEFAULT_BONES_COUNT, 
        0, 
        0
    );

    // romCopy(
    //     ANIM_DATA_ROM_ADDRESS(_character_animationsSegmentRomStart, output_default_bones), 
    //     (void*)objectTest.boneTransforms, 
    //     sizeof(struct Transform) * objectTest.numberOfBones
    // );

    // quatIdent(&objectTest.boneTransforms[0].rotation);

    // objectTest.boneTransforms[1].position.y = 64.0f;

    // skUpdateTransforms(&objectTest);

    osWritebackDCache(objectTest.boneMatrices, sizeof(Mtx) * objectTest.numberOfBones);

    u16* currBuffer = maxMemory;
    
    gInfo[0].msg.gen.type = OS_SC_DONE_MSG;

    currBuffer -= SCREEN_HT * SCREEN_WD;
    gInfo[0].cfb = currBuffer;
    gInfo[1].msg.gen.type = OS_SC_DONE_MSG;
    currBuffer -= SCREEN_HT * SCREEN_WD;
    gInfo[1].cfb = currBuffer;

    /* The Vi manager was started by scheduler by this point in time */
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);

    return currBuffer;
}

void createGfxTask(GFXInfo *i) 
{
    static int firsttime = 1;
    Dynamic *dynamicp;
    OSScTask *t;
    
    /**** pointers to build the display list. ****/
    dynamicp = &i->dp;
    glistp   = i->dp.glist;

    /**** Tell RCP where each segment is  ****/
    gSPSegment(glistp++, 0, 0);	/* physical addressing segment */
    gSPSegment(glistp++, STATIC_SEGMENT,  osVirtualToPhysical(staticSegment));
    gSPSegment(glistp++, DYNAMIC_SEGMENT, osVirtualToPhysical(dynamicp));
    gSPSegment(glistp++, CHARACTER_SEGMENT, osVirtualToPhysical(characterSegment));

    /**** Graphics pipeline state initialization ****/
    gSPDisplayList(glistp++, setup_rspstate);
    if (firsttime) {
        gSPDisplayList(glistp++, rdpstateinit_dl);
	firsttime = 0;
    }

    gSPDisplayList(glistp++, setup_rdpstate);
    
    /**** clear zbuffer, z = max z, dz = 0  ****/
    gDPSetDepthImage(glistp++, osVirtualToPhysical(zbuffer));
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		     osVirtualToPhysical(zbuffer));
    gDPSetFillColor(glistp++, (GPACK_ZDZ(G_MAXFBZ,0) << 16 |
			       GPACK_ZDZ(G_MAXFBZ,0)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	
    /**** Clear framebuffer cvg = FULL or 1  ****/
    gDPPipeSync(glistp++);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		     osVirtualToPhysical(i->cfb));
    /* clearcolor is 32-bits (2 pixels wide) because fill mode
     * does two pixels at a time.
     */
    gDPSetFillColor(glistp++, (GPACK_RGBA5551(0, 0, 0, 1) << 16 | 
			       GPACK_RGBA5551(0, 0, 0, 1)));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE); 

    assert((void *)glistp < (void *)&i->msg);

    /**** Draw objects */
    doLogo(dynamicp);

    
    /**** Put an end on the top-level display list  ****/
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    /* Flush the dynamic segment */
    osWritebackDCache(&i->dp, (s32)glistp - (s32)&i->dp);

    /* build graphics task */

    t = &i->task;
    t->list.t.data_ptr = (u64 *) dynamicp->glist;
    t->list.t.data_size = (s32)(glistp - dynamicp->glist) * sizeof (Gfx);
    t->list.t.type = M_GFXTASK;
    // t->list.t.flags = OS_TASK_LOADABLE;
    t->list.t.flags = 0;
    t->list.t.ucode_boot = (u64 *)rspbootTextStart;
    t->list.t.ucode_boot_size = ((s32) rspbootTextEnd - (s32) rspbootTextStart);
    t->list.t.ucode =      (u64 *) gspF3DEX2_fifoTextStart;
    t->list.t.ucode_data = (u64 *) gspF3DEX2_fifoDataStart; 
    t->list.t.output_buff = (u64 *) rdp_output;
    t->list.t.output_buff_size = (u64 *) rdp_output + 0x4000/8;
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

/*
 * Draw the SGI Logo
 *
 * You can mix the gu and the gSP commands.  The important item to 
 * remember is that the cache must be flushed of any dynamic data
 * before the RSP starts reading the command list.
 */
void doLogo(Dynamic *dynamicp)
{
    u16		   perspNorm;
    static float   logo_theta = 0;
    /*
     * You must make the call to gSPPerspNormalize() in addition to 
     * using the perspective projection matrix.
     */
    guPerspective(&dynamicp->projection, &perspNorm,
		  33, 320.0/240.0, 400, 2000, 1.0);
    gSPPerspNormalize(glistp++, perspNorm);

    guLookAt(&dynamicp->viewing, 
	     0, 0, 1000,
	     0, 0, 0,
	     0, 1, 0);
    

    /* draw the background first */
    guScale(&dynamicp->bg_model, 12.0, 7.0, 1.0);
    gSPMatrix(glistp++, &dynamicp->projection, 
	       G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &dynamicp->viewing, 
	       G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &dynamicp->bg_model, 
	       G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPDisplayList(glistp++, bg_dl);

    float scale = 0.25f;

    /* Position the logo: */
    guTranslate(&dynamicp->logo_trans, logoPos_x*4, logoPos_y*2, logoPos_z);
    /* Scale the logo */
    guScale(&dynamicp->logo_scale, logoScale_x * scale, logoScale_y * scale, logoScale_z * scale);
    /* Rotate the logo */
    guRotate(&dynamicp->logo_rotate, logo_theta, 0.0, 1.0, 0.0);

    /* Setup model matrix */
    gSPMatrix(glistp++, &dynamicp->projection, 
	       G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &dynamicp->viewing, 
	       G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &dynamicp->logo_trans, 
	      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &dynamicp->logo_scale, 
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    gSPMatrix(glistp++, &dynamicp->logo_rotate, 
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    /* Draw the logo */
    gSPDisplayList(glistp++, logo_dl);
    // gSPDisplayList(glistp++, mat_mario_sm64_material);
    // gSPDisplayList(glistp++, mario_Cube_mesh);
    // gDPPipeSync(glistp++);
    // gSPDisplayList(glistp++, mario_Cube_mesh_tri_0);
    // gSPDisplayList(glistp++, output_model_gfx);
    
    // skRenderObject(&objectTest, &glistp);

    /* calculate theta for next frame */
    logo_theta += logoVeloc;
}
