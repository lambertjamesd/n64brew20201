#ifndef _TIME_H
#define _TIME_H

#include <ultra64.h>

extern float gTimeDelta;
extern OSTime gLastTime;

void timeUpdateDelta();

#endif