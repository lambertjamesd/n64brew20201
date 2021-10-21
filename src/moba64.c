
#include <ultra64.h>
#include <sched.h>
#include "moba64.h"
#include "audio/audio.h"
#include "graphics/gfx.h"
#include "../debugger/debugger.h"
#include "util/memory.h"
#include "util/time.h"
#include "scene/scene_management.h"
#include "controls/controller.h"
#include "sk64/skelatool_animator.h"
#include "scene/minion.h"

/**** threads used by this file ****/
static OSThread gameThread;
static OSThread initThread;

/**** Stack for boot code.  Space can be reused after 1st thread starts ****/

/**** Stacks for the threads, divide by 8 which is the size of a u64 ****/
u64    bootStack[STACKSIZEBYTES/sizeof(u64)];
static u64      gameThreadStack[STACKSIZEBYTES/sizeof(u64)];
static u64      initThreadStack[STACKSIZEBYTES/sizeof(u64)];

/**** function prototypes for private functions in this file ****/
static void     gameproc(void *);
static void     initproc(char *);
static void     initGame(void);

/**** message queues and message buffers used by this app ****/
static OSMesg           PiMessages[DMA_QUEUE_SIZE];
static OSMesgQueue      PiMessageQ;
OSMesgQueue dmaMessageQ;
static OSMesg           dmaMessageBuf;

/**** Graphics variables used in this file ****/
OSMesgQueue     gfxFrameMsgQ;
OSMesg          gfxFrameMsgBuf[MAX_MESGS];
GFXInfo         gInfo[2];

/**** Scheduler globals ****/
OSSched         sc;
OSMesgQueue	*sched_cmdQ;
u64             scheduleStack[OS_SC_STACKSIZE/8];
OSScClient      gfxClient;

/**** Controller globals ****/
extern u8      validcontrollers;

OSPiHandle	*gPiHandle;

void boot(void *arg)
{
    u32    i;
    u32    *argp;
    u32    argbuf[16];
    
    osInitialize();

    gPiHandle = osCartRomInit();
    
    osCreateThread(&initThread, 1, (void(*)(void *))initproc, arg,
                  (void *)(initThreadStack+(STACKSIZEBYTES/sizeof(u64))), 
		   (OSPri)INIT_PRIORITY);

    osStartThread(&initThread);
}

static void initproc(char *argv) 
{
    /**** Start PI Mgr for access to cartridge ****/
    osCreatePiManager((OSPri) OS_PRIORITY_PIMGR, &PiMessageQ, PiMessages,
                        DMA_QUEUE_SIZE);

    /**** Create the game thread and start it up ****/
    osCreateThread(&gameThread, 6, gameproc, argv, gameThreadStack + 
		   (STACKSIZEBYTES/sizeof(u64)), (OSPri)GAME_PRIORITY);

    osStartThread(&gameThread);

    /**** Set the thread to be the idle thread ****/
    osSetThreadPri(0, 0);
    for(;;);
}

void debugDrawScreen(u8 r, u8 g, u8 b, u32 frameCount) {
    u32         drawbuffer = 0;
    OSMesgQueue     retraceMessageQ;
    OSMesg          retraceMessageBuf[20];

    osViBlack(0);
	osCreateMesgQueue(&retraceMessageQ, retraceMessageBuf, 20);
	osViSetEvent(&retraceMessageQ, NULL, 1);

    u16 color = (((u16)r & 0x1F) << 11) | (((u16)g & 0x1F) << 6) | (((u16)b & 0x1F) << 1) | 1;

    while (frameCount) {
        for (int y = 0; y < SCREEN_HT; ++y) {
            for (int x = 0; x < SCREEN_WD; ++x) {
                gInfo[drawbuffer].cfb[x + y * SCREEN_WD] = color;
            }
        }

        osWritebackDCacheAll();

        osViSwapBuffer(gInfo[drawbuffer].cfb);

        drawbuffer ^= 1; /* switch the drawbuffer */

        while (!MQ_IS_EMPTY(&retraceMessageQ))
        {
            (void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_NOBLOCK);
        }
        (void) osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);

        frameCount--;
    }
}

static void gameproc(void *argv)
{
    u32         drawbuffer = 0;
    u32         pendingGFX = 0;
    GFXMsg      *msg = NULL;

    initGame();

    while (1) 
    {
        (void) osRecvMesg(&gfxFrameMsgQ, (OSMesg *)&msg, OS_MESG_BLOCK);

        switch (msg->gen.type) 
        {
            case (OS_SC_RETRACE_MSG):
                /**** Create a new gfx task unless we already have 2  ****/                 
                if (pendingGFX < 2) 
                {
                    createGfxTask(&gInfo[drawbuffer]);
                    pendingGFX++;
                    drawbuffer ^= 1; /* switch the drawbuffer */
                }

                controllersTriggerRead();
                timeUpdateDelta();
                skReadMessages();
                sceneUpdate();

                break;

            case (OS_SC_DONE_MSG):
                pendingGFX--;        /* decrement number of pending tasks */
                break;
                
            case SIMPLE_CONTROLLER_MSG:
                controllersUpdate();
                break;
                
            case (OS_SC_PRE_NMI_MSG): /* stop creation of graphics tasks */
                pendingGFX += 2;
                break;
        }
    }
}

extern char     _heapStart[];

static void initGame(void)
{   
    /**** set up a needed message q's ****/
    osCreateMesgQueue(&dmaMessageQ, &dmaMessageBuf, 1);
    osCreateMesgQueue(&gfxFrameMsgQ, gfxFrameMsgBuf, MAX_MESGS);

    /**** Initialize the RCP task scheduler ****/
    osCreateScheduler(&sc, (void *)(scheduleStack + OS_SC_STACKSIZE/8),
                      SCHEDULER_PRIORITY, OS_VI_NTSC_LAN1, NUM_FIELDS);

    /**** Add ourselves to the scheduler to receive retrace messages ****/
    osScAddClient(&sc, &gfxClient, &gfxFrameMsgQ);  

    sched_cmdQ = osScGetCmdQ(&sc);


    /**** Call the initialization routines ****/
    void* heapEnd = initGFXBuffers((void*)PHYS_TO_K0(osMemSize)); 
    initHeap(_heapStart, heapEnd);
    initGFX();
    controllersInit();
    initAudio();
    skInitDataPool(gPiHandle);
    minionSetup();

    OSThread* debugThreads[2];
    debugThreads[0] = &gameThread;
    enum GDBError err = gdbInitDebugger(gPiHandle, &dmaMessageQ, debugThreads, 1);
}

