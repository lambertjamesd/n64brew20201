
#include "controller.h"
#include "game_defs.h"
#include "moba64.h"

static u8    validcontrollers = 0;
static u8    cntrlReadInProg  = 0;

static OSContStatus  gControllerStatus[MAX_PLAYERS];
static OSContPad     gControllerData[MAX_PLAYERS];
static OSScMsg       gControllerMessage;
static u16           gControllerLastButton[MAX_PLAYERS];

void controllersInit(void)
{
    OSMesgQueue         serialMsgQ;
    OSMesg              serialMsg;
    s32                 i;

    osCreateMesgQueue(&serialMsgQ, &serialMsg, 1);
    osSetEventMesg(OS_EVENT_SI, &serialMsgQ, (OSMesg)1);

    if((i = osContInit(&serialMsgQ, &validcontrollers, &gControllerStatus[0])) != 0)
        PRINTF("Failure initing controllers\n");
    
    /**** Set up message and queue, for read completion notification ****/
    gControllerMessage.type = SIMPLE_CONTROLLER_MSG;

    osSetEventMesg(OS_EVENT_SI, &gfxFrameMsgQ, (OSMesg)&gControllerMessage);

}

void controllersUpdate(void)
{
    gControllerLastButton[0] = gControllerData[0].button;
    gControllerLastButton[1] = gControllerData[1].button;
    gControllerLastButton[2] = gControllerData[2].button;
    gControllerLastButton[3] = gControllerData[3].button;

    osContGetReadData(gControllerData);
    cntrlReadInProg = 0;
}

void controllersTriggerRead(void) {
    if (validcontrollers && !cntrlReadInProg)
    {
        cntrlReadInProg = 1;
        osContStartReadData(&gfxFrameMsgQ);
    }
}

OSContPad* controllersGetControllerData(int index) {
    return &gControllerData[index];
}