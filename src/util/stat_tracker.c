#include "stat_tracker.h"

#include "math/mathf.h"

void statTrackerInit(struct StatTracker* tracker) {
    tracker->nextOut = 0;
    tracker->currentStep = 1.0f;

    for (unsigned i = 0; i < STAT_MAX_RESOLUTION; ++i) {
        tracker->data[i] = 0.0f;
    }
}

void statTrackerLogDatapoint(struct StatTracker* tracker, float datapoint) {
    tracker->data[(int)tracker->nextOut] += datapoint * tracker->currentStep;
    tracker->nextOut += tracker->currentStep;

    // when the data is full compress it down
    if (tracker->nextOut >= STAT_MAX_RESOLUTION) {
        unsigned src = 0;
        for (unsigned i = 0; i < STAT_MAX_RESOLUTION / 2; ++i) {
            tracker->data[i] = (tracker->data[src] + tracker->data[src+1]) * 0.5f;
            src += 2;
        }
        for (unsigned i = STAT_MAX_RESOLUTION / 2; i < STAT_MAX_RESOLUTION; ++i) {
            tracker->data[i] = 0.0f;
        }
        tracker->nextOut *= 0.5f;
        tracker->currentStep *= 0.5f;
    }
}

void statTrackerFinalize(struct StatTracker* tracker) {
    float floored = floorf(tracker->nextOut);

    if (floored != tracker->nextOut) {
        tracker->data[(int)floored] /= (tracker->nextOut - floored);
        tracker->nextOut = ceilf(tracker->nextOut);
    }
}

float statTrackerReadSample(struct StatTracker* tracker, float index) {
    unsigned arrayIndex = (unsigned)index;

    if (arrayIndex + 2 >= tracker->nextOut) {
        return tracker->data[(int)tracker->nextOut - 1];
    }

    float prevSample = tracker->data[arrayIndex];
    float nextSample = tracker->data[arrayIndex + 1];
    float lerp = index - arrayIndex;;

    return prevSample * (1.0f - lerp) + nextSample * lerp;
}

void statTrackerWriteResults(struct StatTracker* tracker, float* to, unsigned outputSize) {
    for (unsigned i = 0; i < outputSize; ++i) {
        to[i] = floorf(statTrackerReadSample(tracker, i * tracker->nextOut / (float)outputSize) + 0.5f);
    }
}