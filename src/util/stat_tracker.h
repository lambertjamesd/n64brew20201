#ifndef STAT_TRACKER_H
#define STAT_TRACKER_H

#define STAT_MAX_RESOLUTION     64

struct StatTracker {
    float data[STAT_MAX_RESOLUTION];
    float nextOut;
    float currentStep;
};

void statTrackerInit(struct StatTracker* tracker);
void statTrackerLogDatapoint(struct StatTracker* tracker, float datapoint);
void statTrackerFinalize(struct StatTracker* tracker);
void statTrackerWriteResults(struct StatTracker* tracker, float* to, unsigned outputSize);

#endif