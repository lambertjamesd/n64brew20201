
#include "endgamemenu.h"

struct StatTracker gPlayerBaseStats[MAX_PLAYERS];

void endGameMenuResetStats() {
    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        statTrackerInit(&gPlayerBaseStats[i]);
    }
}