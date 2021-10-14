
#include <sched.h>
#include "moba64.h"

u8    validcontrollers = 0; 

static OSContStatus  statusdata[MAXCONTROLLERS];
static OSContPad     controllerdata[MAXCONTROLLERS];
static OSScMsg       controllermsg;
static u16           lastButArray[MAXCONTROLLERS];

extern	u32	ucode_index;

void initCntrl(void)
{
    OSMesgQueue         serialMsgQ;
    OSMesg              serialMsg;
    s32                 i;

    osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
    osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);

    if((i = osContInit(&serialMsgQ, &validcontrollers, &statusdata[0])) != 0)
        PRINTF("Failure initing controllers\n");
    
    /**** Set up message and queue, for read completion notification ****/
    controllermsg.type = SIMPLE_CONTROLLER_MSG;

    osSetEventMesg(OS_EVENT_SI, &gfxFrameMsgQ, (OSMesg)&controllermsg);

}

void UpdateController(void)
{
    osContGetReadData(controllerdata);
}

