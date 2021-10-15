/*
 * moba64.h
 */

#ifndef __simple__
#define __simple__

#define	STACKSIZEBYTES	0x2000

#define	STATIC_SEGMENT		1

#define CHARACTER_SEGMENT   3
#define LEVEL_SEGMENT   4

#ifdef _LANGUAGE_C

#include <ultra64.h>
#include "graphics/gfx.h"

#define MAX_MESGS	8
#define DMA_QUEUE_SIZE  200
#define NUM_FIELDS      1       /* 1 = 60 Hz video interrupts, 2 = 30 Hz, etc */

#define UTIL_PRIORITY		1
#define INIT_PRIORITY		10
#define GAME_PRIORITY		10
#define AUDIO_PRIORITY		12
#define SCHEDULER_PRIORITY	13

#define LOG_SCHEDULE_GFX_TASK	    101
#define LOG_RDP_DONE		    102
#define LOG_RETRACE		    103
#define LOG_INTR		    104

/* define a message after the set used by the scheduler */
#define SIMPLE_CONTROLLER_MSG	    (OS_SC_LAST_MSG+1)

#define PRINTF(a) 

extern OSMesgQueue      gfxFrameMsgQ;
extern OSMesg           gfxFrameMsgBuf[MAX_MESGS];
extern OSPiHandle	*handler;

extern char _staticSegmentRomStart[], _staticSegmentRomEnd[];
extern char _tableSegmentRomStart[], _tableSegmentRomEnd[];
extern char _seqSegmentRomStart[], _seqSegmentRomEnd[];
extern char _bankSegmentRomStart[], _bankSegmentRomEnd[];


void     initAudio(void);
void    initGFX();
void*    initGFXBuffers(void* maxMemory);
void     initCntrl(void);
void     createGfxTask(GFXInfo *i);
void     UpdateController(void);

#endif /* _LANGUAGE_C */

#endif


