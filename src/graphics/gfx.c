
#include <assert.h>

#include "gfx.h"
#include "moba64.h"
#include "util/memory.h"
#include "sk64/skelatool_armature.h"
#include "sk64/skelatool_defs.h"
#include "util/rom.h"
#include "sprite.h"
#include "sk64/skelatool_defs.h"

#include "scene/scene_management.h"

/*
 * graphics globals
 */

extern OSSched         sc;
extern OSMesgQueue     *sched_cmdQ;
extern GFXInfo         gInfo[];

OSTime gGFXCreateTime;
OSTime gUpdateTime;
OSTime gGFXRSPTime;

char         *gStaticSegment = 0;
char         *gMenuSegment = 0;
char         *gCharacterSegment = 0;
char         *gLevelSegment = 0;
char         *gThemeSegment = 0;
char         *gFontSegment = 0;
char         *gWireframeSegment = 0;

void* rdp_output;

Vp gFullScreenVP = {
  .vp = {
    .vscale = {SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0},	/* scale */
    .vtrans = {SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0},	/* translate */
  }
};

struct ViewportLayout gViewportPosition[] = {
    // Single player
    {
        .viewportLocations = {
            {0, 0, SCREEN_WD, SCREEN_HT},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
        },
        .minimapLocation = {SCREEN_WD - MINIMAP_SIZE - 32, SCREEN_HT - MINIMAP_SIZE - 32, SCREEN_WD - 32, SCREEN_HT - 32},
    },
    // Two player
    {
        .viewportLocations = {
            {0, 0, SCREEN_WD/2-1, SCREEN_HT},
            {SCREEN_WD/2+1, 0, SCREEN_WD, SCREEN_HT},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
        },
        .minimapLocation = {(SCREEN_WD - MINIMAP_SIZE) / 2, SCREEN_HT - MINIMAP_SIZE - 32, (SCREEN_WD + MINIMAP_SIZE) / 2, SCREEN_HT - 32},
    },
    // Three player
    {
        .viewportLocations = {
            {0, 0, SCREEN_WD/2-1, SCREEN_HT/2-1},
            {SCREEN_WD/2+1, 0, SCREEN_WD, SCREEN_HT/2-1},
            {0, SCREEN_HT/2+1, SCREEN_WD/2-1, SCREEN_HT},
            {0, 0, 0, 0},
        },
        .minimapLocation = {SCREEN_WD * 3 / 4 - SCREEN_HT / 4 + 16, SCREEN_HT / 2 + 16, SCREEN_WD * 3 / 4 + SCREEN_HT / 4 - 16, SCREEN_HT - 16},
    },
    // Four player
    {
        .viewportLocations = {
            {0, 0, SCREEN_WD/2-1, SCREEN_HT/2-1},
            {SCREEN_WD/2+1, 0, SCREEN_WD, SCREEN_HT/2-1},
            {0, SCREEN_HT/2+1, SCREEN_WD/2-1, SCREEN_HT},
            {SCREEN_WD/2+1, SCREEN_HT/2+1, SCREEN_WD, SCREEN_HT},
        },
        .minimapLocation = {(SCREEN_WD - MINIMAP_SIZE) / 2, (SCREEN_HT - MINIMAP_SIZE) / 2, (SCREEN_WD + MINIMAP_SIZE) / 2, (SCREEN_HT + MINIMAP_SIZE) / 2},
    },
};

Vp gSplitScreenViewports[4];
unsigned short gClippingRegions[4 * 4];

unsigned short	__attribute__((aligned(64))) zbuffer[SCREEN_WD*SCREEN_HT];
u64 __attribute__((aligned(16))) dram_stack[SP_DRAM_STACK_SIZE64];
u64 __attribute__((aligned(16))) gfxYieldBuf2[OS_YIELD_DATA_SIZE/sizeof(u64)];

void initGFX() 
{ 
    skSetSegmentLocation(CHARACTER_ANIMATION_SEGMENT, (unsigned)_character_animationsSegmentRomStart);
    
    gInfo[0].msg.gen.type = OS_SC_DONE_MSG;
    gInfo[1].msg.gen.type = OS_SC_DONE_MSG;
    /* The Vi manager was started by scheduler by this point in time */
    osViSetSpecialFeatures(OS_VI_DITHER_FILTER_ON);
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
    gSPSegment(renderState->dl++, MENU_SEGMENT, osVirtualToPhysical(gMenuSegment));
    gSPSegment(renderState->dl++, CHARACTER_SEGMENT, osVirtualToPhysical(gCharacterSegment));
    gSPSegment(renderState->dl++, LEVEL_SEGMENT, osVirtualToPhysical(gLevelSegment));
    gSPSegment(renderState->dl++, THEME_SEGMENT, osVirtualToPhysical(gThemeSegment));
    gSPSegment(renderState->dl++, FONT_SEGMENT, osVirtualToPhysical(gFontSegment));
    gSPSegment(renderState->dl++, LEVEL_WIREFRAME_SEGMENT, osVirtualToPhysical(gWireframeSegment));

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
    gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, osVirtualToPhysical(zbuffer));
    gDPSetFillColor(renderState->dl++, (GPACK_ZDZ(G_MAXFBZ,0) << 16 | GPACK_ZDZ(G_MAXFBZ,0)));
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
	
    /**** Clear framebuffer cvg = FULL or 1  ****/
    gDPPipeSync(renderState->dl++);
    gDPSetColorImage(renderState->dl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		     osVirtualToPhysical(i->cfb));
    /* clearcolor is 32-bits (2 pixels wide) because fill mode
     * does two pixels at a time.
     */
    // gDPSetFillColor(renderState->dl++, (GPACK_RGBA5551(0, 0, 0, 1) << 16 | 
	// 		       GPACK_RGBA5551(0, 0, 0, 1)));
    // gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE); 

    /**** Draw objects */
    sceneRender(renderState);
    
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

void gfxInitSplitscreenViewport(unsigned playercount) {
    // 4 numbers per viewport, 4 viewports per slot
    struct ViewportLayout* viewprtLayout = &gViewportPosition[playercount - 1];
    
    for (unsigned i = 0; i < playercount; ++i) {
        unsigned l = viewprtLayout->viewportLocations[i][0];
        unsigned t = viewprtLayout->viewportLocations[i][1];
        unsigned r = viewprtLayout->viewportLocations[i][2];
        unsigned b = viewprtLayout->viewportLocations[i][3];

        gSplitScreenViewports[i].vp.vscale[0] = (r - l) * 4 / 2;
        gSplitScreenViewports[i].vp.vscale[1] = (b - t) * 4 / 2;
        gSplitScreenViewports[i].vp.vscale[2] = G_MAXZ/2;
        gSplitScreenViewports[i].vp.vscale[3] = 0;

        gSplitScreenViewports[i].vp.vtrans[0] = (r + l) * 4 / 2;
        gSplitScreenViewports[i].vp.vtrans[1] = (b + t) * 4 / 2;
        gSplitScreenViewports[i].vp.vtrans[2] = G_MAXZ/2;
        gSplitScreenViewports[i].vp.vtrans[3] = 0;

        gClippingRegions[i * 4 + 0] = l;
        gClippingRegions[i * 4 + 1] = t;
        gClippingRegions[i * 4 + 2] = r;
        gClippingRegions[i * 4 + 3] = b;
    }
}

struct Coloru8 gfxCreateColor = {255, 0, 0, 255};
struct Coloru8 updateColor = {0, 255, 0, 255};
struct Coloru8 rspColor = {0, 0, 255, 255};

#define BAR_Y 32
#define BAR_X 32
#define BAR_W 120

#define TARGET_FRAME_USECS  33333

void gfxDrawTimingInfo(struct RenderState* renderState) {

    unsigned createWidth = BAR_W * OS_CYCLES_TO_USEC(gGFXCreateTime) / TARGET_FRAME_USECS;
    unsigned updateWidth = BAR_W * OS_CYCLES_TO_USEC(gUpdateTime) / TARGET_FRAME_USECS;
    unsigned rspWidth = BAR_W * OS_CYCLES_TO_USEC(gGFXRSPTime) / TARGET_FRAME_USECS;

    spriteSetColor(renderState, LAYER_SOLID_COLOR, gfxCreateColor);
    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X, BAR_Y, createWidth, 4);

    spriteSetColor(renderState, LAYER_SOLID_COLOR, updateColor);
    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X + createWidth, BAR_Y + 4, updateWidth, 4);

    spriteSetColor(renderState, LAYER_SOLID_COLOR, rspColor);
    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X + createWidth, BAR_Y + 8, rspWidth, 4);

    spriteSetColor(renderState, LAYER_SOLID_COLOR, gColorWhite);
    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X + BAR_W, BAR_Y, 2, 12);

    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X + BAR_W * 30 / 60, BAR_Y, 2, 12);
    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X + BAR_W * 30 / 20, BAR_Y, 2, 12);
    spriteSolid(renderState, LAYER_SOLID_COLOR, BAR_X + BAR_W * 30 / 15, BAR_Y, 2, 12);
}