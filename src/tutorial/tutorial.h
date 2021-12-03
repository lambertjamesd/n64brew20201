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
    TutorialStateSpinAttack,
    TutorialStateSpinAttackRelease,
    TutorialStateCapture,
    TutorialStateBaseMenu,
    TutorialStateBaseMenuAttack,
    TutorialStateCollectMinions,
    TutorialStateOrderMinions,
    TutorialStateWin,
    TutorialStateUpgrade,
    TutorialStateUpgradeBuild,
    TutorialStateItemDrop,
    TutorialStateDone,
};

enum TutorailStepFlags {
    TutorailStepFlagsMoveToBase = (1 << 0),
    TutorailStepFlagsMoveToMinion = (1 << 1),
    TutorailStepFlagsMoveToItem = (1 << 2),
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

void tutorialInit(struct LevelScene* level, enum TutorialState startWith);
void tutorialUpdate(struct LevelScene* level, struct PlayerInput* input);
void tutorialRender(struct LevelScene* level, struct RenderState* renderState);

#endif