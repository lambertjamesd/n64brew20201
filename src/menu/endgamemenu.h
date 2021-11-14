#ifndef _END_GAME_MENU_H
#define _END_GAME_MENU_H

#include "graphics/render_state.h"
#include "util/stat_tracker.h"
#include "game_defs.h"
#include "audio/soundplayer.h"
#include "sk64/skelatool_animator.h"
#include "sk64/skelatool_armature.h"

extern struct StatTracker gPlayerBaseStats[MAX_PLAYERS];

#define STAT_COLUMNS    32

void endGameMenuResetStats();

enum EndGameState {
    EndGameStateLoading,
    EndGameStateLoaded,
};

struct EndGameMenu {
    unsigned short winningTeam;
    unsigned short teamCount;
    unsigned short state;
    float baseStats[MAX_PLAYERS][STAT_COLUMNS];
    float drawAnimationTimer;
    float maxBases;
    SoundID captureSound;
    struct SKAnimator winnerAnimator;
    struct SKArmature winnerArmature;
    struct Transform winnerTransform;
};

void endGameMenuInit(struct EndGameMenu* menu, unsigned winningTeam, unsigned teamCount);
void endGameMenuRender(struct EndGameMenu* menu, struct RenderState* renderState);
int endGameMenuUpdate(struct EndGameMenu* menu);

#endif