
#include "time.h"

#define MAX_DELTA_TIME      0.1f

float gTimeDelta;
OSTime gLastTime;

void timeUpdateDelta() {
    OSTime currTime = osGetTime();
    OSTime timeDelta = currTime - gLastTime;

    gTimeDelta = (float)OS_CYCLES_TO_USEC(timeDelta) / 1000000.0f;

    gLastTime = currTime;

}