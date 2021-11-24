
#include <ultra64.h>
#include <sched.h>
#include "moba64.h"
#include "audio/audio.h"
#include "graphics/gfx.h"
#ifdef WITH_DEBUGGER
#include "../debugger/debugger.h"
#endif
#include "util/memory.h"
#include "util/time.h"
#include "scene/scene_management.h"
#include "controls/controller.h"
#include "sk64/skelatool_animator.h"
#include "scene/minion.h"
#include "audio/soundplayer.h"
#include "scene/scene_management.h"
#include "audio/dynamic_music.h"
#include "scene/minion_animations.h"
#include "scene/faction.h"
#include "savefile/savefile.h"

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

static void gameproc(void *argv)
{
    u32         drawbuffer = 0;
    u32         useTimer = 0;
    u32         pendingGFX = 0;
    GFXMsg      *msg = NULL;

    OSTime rcpTaskStart[2];

    initGame();

    timeUpdateDelta();

    while (1) 
    {
        (void) osRecvMesg(&gfxFrameMsgQ, (OSMesg *)&msg, OS_MESG_BLOCK);

        switch (msg->gen.type) 
        {
            case (OS_SC_RETRACE_MSG):
                /**** Create a new gfx task unless we already have 2  ****/                 
                if (pendingGFX < 2 && !sceneIsLoading()) 
                {
                    OSTime drawTimeStart = osGetTime();
                    createGfxTask(&gInfo[drawbuffer]);
                    pendingGFX++;
                    rcpTaskStart[drawbuffer] = osGetTime();
                    gGFXCreateTime = rcpTaskStart[drawbuffer] - drawTimeStart;
                    drawbuffer ^= 1; /* switch the drawbuffer */

                }

                OSTime updateTimeStart = osGetTime();
                if (!controllerHasPendingMessage() && pendingGFX == 0) {
                    // save when there isn't a 
                    // message expected on the queue
                    saveFileCheckSave();
                }
                if (!sceneIsLoading()) {
                    controllersTriggerRead();
                }
                timeUpdateDelta();
                skReadMessages();
                sceneUpdate(pendingGFX == 0 && !gShouldSave);
                soundPlayerUpdate();
                dynamicMusicUpdate();

                gUpdateTime = osGetTime() - updateTimeStart;

                break;

            case (OS_SC_DONE_MSG):
                gGFXRSPTime = osGetTime() - rcpTaskStart[useTimer];
                useTimer ^= 1;
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

void* layoutMemory(void* maxMemory) {
    u16* currBuffer = maxMemory;
    currBuffer -= SCREEN_HT * SCREEN_WD;
    gInfo[0].cfb = currBuffer;
    currBuffer -= SCREEN_HT * SCREEN_WD;
    gInfo[1].cfb = currBuffer;

    currBuffer -= AUDIO_HEAP_SIZE / sizeof(u16);
    gAudioHeapBuffer = (u8*)currBuffer;

    currBuffer -= (RDP_OUTPUT_SIZE + 16) / sizeof(u16);
    rdp_output = currBuffer;

    return currBuffer;
}

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
    char* heapEnd = layoutMemory((void*)PHYS_TO_K0(osMemSize)); 
    heapInit(_heapStart, heapEnd);
    minionAnimationsInit();
    factionGlobalInit();
    skInitDataPool(gPiHandle);
    initGFX();
    controllersInit();
    saveFileLoad();
    initAudio();
    soundPlayerInit();
    mainMenuInitSelections(&gMainMenu);
    sceneQueueMainMenu();

#ifdef WITH_DEBUGGER
    OSThread* debugThreads[2];
    debugThreads[0] = &gameThread;
    gdbInitDebugger(gPiHandle, &dmaMessageQ, debugThreads, 1);
#endif
}

