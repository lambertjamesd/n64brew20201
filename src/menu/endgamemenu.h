#ifndef _END_GAME_MENU_H
#define _END_GAME_MENU_H

#include "util/stat_tracker.h"
#include "game_defs.h"

extern struct StatTracker gPlayerBaseStats[MAX_PLAYERS];

void endGameMenuResetStats();

#endif