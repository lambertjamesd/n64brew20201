
#include "endgamemenu.h"
#include "scene/team_data.h"
#include "graphics/sprite.h"
#include "controls/controller.h"
#include "util/time.h"
#include "math/mathf.h"
#include "audio/clips.h"
#include "scene/faction.h"
#include "util/rom.h"
#include "graphics/gfx.h"

#define DRAW_ANIMATION_TIME 3.0f

struct StatTracker gPlayerBaseStats[MAX_PLAYERS];

struct Coloru8 gStripeColor = {128, 128, 128, 255};

struct Transform gWinnerTransform = {
    {0.9f * SCENE_SCALE, -0.6f * SCENE_SCALE, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.5f, 0.5f, 0.5f},
};

void endGameMenuResetStats() {
    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        statTrackerInit(&gPlayerBaseStats[i]);
    }
}

void endGameMenuInit(struct EndGameMenu* menu, unsigned winningTeam, unsigned teamCount) {
    menu->winningTeam = winningTeam;
    menu->teamCount = teamCount;
    menu->drawAnimationTimer = 0.0f;
    menu->maxBases = 0.0f;
    menu->state = EndGameStateLoading;
    menu->captureSound = SOUND_ID_NONE;
    
    for (unsigned teamIndex = 0; teamIndex < teamCount; ++teamIndex) {
        statTrackerFinalize(&gPlayerBaseStats[teamIndex]);
        statTrackerWriteResults(&gPlayerBaseStats[teamIndex], menu->baseStats[teamIndex], STAT_COLUMNS);
    }

    menu->maxBases = 0.0f;
    for (unsigned sampleIndex = 0; sampleIndex < STAT_COLUMNS; ++sampleIndex) {
        float controlledBases = 0.0f;
        for (unsigned teamIndex = 0; teamIndex < teamCount; ++teamIndex) {
            controlledBases += menu->baseStats[teamIndex][sampleIndex];
        }
        menu->maxBases = MAX(controlledBases, menu->maxBases);
    }
}

void endGameDrawGraph(struct EndGameMenu* menu, struct RenderState* renderState, int left, int top, int right, int bottom) {
    unsigned short currentColumnY[STAT_COLUMNS];
    float columnWidth = (float)(right - left) / (float)STAT_COLUMNS;
    
    float heightScalar = 1.0f;

    if (menu->maxBases > 0.0f) {
        heightScalar = (float)(bottom - top) / menu->maxBases;
    }

    for (unsigned i = 0; i < STAT_COLUMNS; ++i) {
        currentColumnY[i] = (float)bottom;
    }

    float finalIndex;
    
    if (menu->state == EndGameStateLoading) {
        finalIndex = menu->drawAnimationTimer * STAT_COLUMNS / DRAW_ANIMATION_TIME;
    } else {
        finalIndex = STAT_COLUMNS;
    }

    spriteSetColor(renderState, LAYER_SOLID_COLOR, gStripeColor);

    float currY = (float)bottom;

    for (unsigned sliceIndex = 0; sliceIndex < (int)menu->maxBases; ++sliceIndex) {
        spriteSolid(
            renderState,
            LAYER_SOLID_COLOR,
            left,
            (int)currY + 1,
            right - left,
            1
        );

        currY -= heightScalar;
    }

    for (unsigned teamIndex = 0; teamIndex < menu->teamCount; ++teamIndex) {
        float x = left;

        spriteSetColor(renderState, LAYER_SOLID_COLOR, gTeamColors[teamIndex]);
        for (unsigned sampleIndex = 0; sampleIndex < STAT_COLUMNS && sampleIndex < finalIndex + 1; ++sampleIndex) {
            float scaledHeight = heightScalar * menu->baseStats[teamIndex][sampleIndex];

            if (sampleIndex >= finalIndex) {
                scaledHeight *= finalIndex + 1 - sampleIndex;
            }
            
            unsigned short height = (unsigned short)scaledHeight;

            currentColumnY[sampleIndex] -= height;

            spriteSolid(
                renderState, 
                LAYER_SOLID_COLOR,
                (int)x,
                currentColumnY[sampleIndex],
                (int)columnWidth - 1,
                height
            );

            x += columnWidth;
        }
    }
}

void endGameMenuRender(struct EndGameMenu* menu, struct RenderState* renderState) {
    spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
    spriteSolid(renderState, LAYER_SOLID_COLOR, 20, 30, 180, 180);
    endGameDrawGraph(menu, renderState, 30, 40, 190, 200);

    if (menu->state == EndGameStateLoaded) {
        Mtx* matrix = renderStateRequestMatrices(renderState, 1);
        transformToMatrixL(&menu->winnerTransform, matrix);
        gSPMatrix(renderState->dl++, matrix, G_MTX_PUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        gSPDisplayList(renderState->dl++, gTeamPalleteTexture[menu->winningTeam]);
        skRenderObject(&menu->winnerArmature, renderState);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }
}

float endGameCalcCaptureFreq(struct EndGameMenu* menu) {
    unsigned progressIndex = (unsigned)(menu->drawAnimationTimer * STAT_COLUMNS / DRAW_ANIMATION_TIME);

    if (progressIndex + 1 >= STAT_COLUMNS) {
        progressIndex = STAT_COLUMNS - 1;
    }

    float activeBases = 0.0f;

    for (unsigned i = 0; i < menu->teamCount; ++i) {
        activeBases += menu->baseStats[i][progressIndex];
    }

    return powf(2.0f, activeBases / menu->maxBases) * 0.5f;    
}

void endGameMenuEnterLoadedState(struct EndGameMenu* menu) {
    menu->state = EndGameStateLoaded;
    soundPlayerStop(&menu->captureSound);
    menu->winnerTransform = gWinnerTransform;

    struct Faction* winningFaction = gTeamFactions[menu->winningTeam];

    skAnimatorInit(&menu->winnerAnimator, winningFaction->playerBoneCount, 0, 0);
    skArmatureInit(
        &menu->winnerArmature, 
        winningFaction->playerMesh, 
        winningFaction->playerBoneCount, 
        CALC_ROM_POINTER(character_animations, winningFaction->playerDefaultPose),
        winningFaction->playerBoneParent
    );
    skAnimatorRunClip(&menu->winnerAnimator, winningFaction->playerAnimations[PlayerAnimationSelected], 0);
}

int endGameMenuUpdate(struct EndGameMenu* menu) {
    if (controllerGetButtonDown(0, B_BUTTON | START_BUTTON | A_BUTTON)) {
        soundPlayerStop(&menu->captureSound);
        return 1;
    }

    switch (menu->state) {
        case EndGameStateLoading:
            if (menu->captureSound == SOUND_ID_NONE) {
                menu->captureSound = soundPlayerPlay(SOUNDS_FLAGCAP, SoundPlayerFlagsLoop);
            }

            menu->drawAnimationTimer += gTimeDelta;

            soundPlayerSetPitch(menu->captureSound, endGameCalcCaptureFreq(menu));

            if (menu->drawAnimationTimer > DRAW_ANIMATION_TIME) {
                menu->drawAnimationTimer = DRAW_ANIMATION_TIME;
                endGameMenuEnterLoadedState(menu);
            }
            break;
        case EndGameStateLoaded:
            skAnimatorUpdate(&menu->winnerAnimator, menu->winnerArmature.boneTransforms, 1.0f);
            break;
    }

    return 0;
}