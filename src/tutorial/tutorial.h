#ifndef _TUTORIAL_H
#define _TUTORIAL_H

#include "menu/textbox.h"
#include "scene/level_scene.h"
#include "graphics/render_state.h"
#include "graphics/sprite.h"
#include "scene/playerinput.h"

enum TutorialState {
    TutorialStateMove,
    TutorialStateJump,
    TutorialStateAttack,
    TutorialStateCapture,
    TutorialStateBaseMenu,
    TutorialStateCollectMinions,
    TutorialStateOrderMinions,
    TutorialStateWin,
    TutorialStateDone,
};

enum TutorailStepFlags {
    TutorailStepFlagsMoveToBase = (1 << 0),
    TutorailStepFlagsMoveToMinion = (1 << 1),
};

struct TutorialStep {
    char* message;
    struct SpriteTile icon;
    unsigned short flags;
    unsigned short team;
};


struct Tutorial {
    enum TutorialState currentState;
    enum TutorialState nextState;
    struct TextBox textBox;
    float waitForActionTimer;
};

void tutorialInit(struct LevelScene* level);
void tutorialUpdate(struct LevelScene* level, struct PlayerInput* input);
void tutorialRender(struct LevelScene* level, struct RenderState* renderState);

#endif