#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <ultra64.h>

void controllersInit(void);
void controllersUpdate(void);
void controllersTriggerRead(void);

OSContPad* controllersGetControllerData(int index);
u16 controllerGetLastButton(int index);

#endif