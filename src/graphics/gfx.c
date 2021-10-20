
#include <assert.h>

#include "gfx.h"
#include "moba64.h"
#include "gfxvalidator/validator.h"
#include "gfxvalidator/error_printer.h"
#include "util/memory.h"
#include "../data/models/example/geometry.h"
#include "sk64/skelatool_armature.h"
#include "sk64/skelatool_defs.h"
#include "util/rom.h"


#include "scene/scene_management.h"

/*
 * graphics globals
 */

extern OSSched         sc;
extern OSMesgQueue     *sched_cmdQ;
extern GFXInfo         gInfo[];

char         *gStaticSegment = 0;
char         *gCharacterSegment = 0;
char         *gLevelSegment = 0;

#define RDP_OUTPUT_SIZE 0x4000
static void* rdp_output;

unsigned short	__attribute__((aligned(64))) zbuffer[SCREEN_WD*SCREEN_HT];
u64 __attribute__((aligned(16))) dram_stack[SP_DRAM_STACK_SIZE64];
u64 __attribute__((aligned(16))) gfxYieldBuf2[OS_YIELD_DATA_SIZE/sizeof(u64)];

void initGFX() 
{    
    u32 len = (u32)(_staticSegmentRomEnd - _staticSegmentRomStart);

    assert (len < GFX_DL_BUF_SIZE * sizeof(Gfx));
    gStaticSegment = malloc(len);
    romCopy(_staticSegmentRomStart, gStaticSegment, len);

    LOAD_SEGMENT(characters, gCharacterSegment);
    LOAD_SEGMENT(level_test, gLevelSegment);

    loadLevelScene();
    
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

    currBuffer -= (RDP_OUTPUT_SIZE + 16) / sizeof(u16);
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
    gSPSegment(renderState->dl++, STATIC_SEGMENT,  osVirtualToPhysical(gStaticSegment));
    gSPSegment(renderState->dl++, CHARACTER_SEGMENT, osVirtualToPhysical(gCharacterSegment));
    gSPSegment(renderState->dl++, LEVEL_SEGMENT, osVirtualToPhysical(gLevelSegment));

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
    levelSceneRender(&gCurrentLevel, renderState);
    
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
    t->list.t.output_buff_size = ((u64 *) rdp_output + RDP_OUTPUT_SIZE/sizeof(u64));
    t->list.t.ucode_data_size = SP_UCODE_DATA_SIZE;
    // t->list.t.ucode_size = 0;
    t->list.t.dram_stack = (u64 *) dram_stack;
    t->list.t.dram_stack_size = SP_DRAM_STACK_SIZE8;
    t->list.t.yield_data_ptr = (u64 *) gfxYieldBuf2;
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
}