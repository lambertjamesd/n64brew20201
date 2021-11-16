
#include "tutorial.h"

#include "graphics/gfx.h"
#include "../data/gameplaymenu/menu.h"
#include "graphics/sprite.h"
#include "util/time.h"
#include "scene/events.h"

#define TUTORIAL_Y 190

#define WAIT_FOR_ACTION_TIME 2.0f
#define ARROW_FLICKER_TIME  0.5f

#define ON_BASE_RADIUS  (1.5f * SCENE_SCALE)

struct Tutorial gTutorial;

struct TutorialStep gTutorialStep[] = {
    {
        "Move",
        {16, 48, 16, 16},
        0,
        0,
    },
    {
        "Jump",
        {0, 0, 16, 16},
        0,
        0,
    },
    {
        "Attack",
        {16, 0, 16, 16},
        0,
        0,
    },
    {
        "Capture Base",
        {0, 0, 0, 0},
        TutorailStepFlagsMoveToBase,
        TEAM_NONE,
    },
    {
        "Base Orders",
        {0, 16, 16, 16},
        TutorailStepFlagsMoveToBase,
        TEAM(0),
    },
    {
        "Collect Bots",
        {16, 32, 16, 16},
        TutorailStepFlagsMoveToMinion,
        TEAM(0),
    },
    {
        "Order Bots",
        {0, 32, 16, 16},
        0,
        0,
    },
    {
        "Kick Butt",
        {0, 0, 0, 0},
        TutorailStepFlagsMoveToBase,
        TEAM(1),
    },
    {
        "",
        {0, 0, 0, 0},
        0,
        0,
    },
};

int tutorialIconRenderer(struct RenderState* renderState, void* data, int x, int y) {
    struct SpriteTile* tile = &gTutorialStep[gTutorial.currentState].icon;

    if (tile->w == 0) {
        return 0;
    }

    if (renderState) {
        spriteDrawTile(renderState, LAYER_BUTTONS, x, y, tile->w, tile->h, *tile);
    }

    return tile->w + 8;
}

void tutorialInit(struct LevelScene* level) {
    textBoxInitEmpty(&gTutorial.textBox);
    gTutorial.currentState = TutorialStateMove;
    gTutorial.nextState = TutorialStateMove;
    gTutorial.waitForActionTimer = WAIT_FOR_ACTION_TIME;
}

struct LevelBase* tutorialGetBaseForTeam(struct LevelScene* level, struct Vector3* near, unsigned team) {
    struct LevelBase* result = 0;
    float distance = 0;

    for (unsigned i = 0; i < level->baseCount; ++i) {
        struct LevelBase* curr = &level->bases[i];

        if (levelBaseGetTeam(curr) != team) {
            continue;
        }

        float currDistance = vector3DistSqrd(&curr->position, near);

        if (result == 0 || currDistance < distance) {
            distance = currDistance;
            result = curr;
        }
    }

    return result;
}

struct Minion* tutorialGetMinion(struct LevelScene* level, unsigned team, int command) {
    for (unsigned i = 0; i < level->minionCount; ++i) {
        if (minionIsAlive(&level->minions[i]) && level->minions[i].team.teamNumber == team) {
            if (command == -1 || level->minions[i].currentCommand == command) {
                return &level->minions[i];
            }
        }
    }

    return 0;
}

void tutorialUpdate(struct LevelScene* level, struct PlayerInput* input) {
    if (gTutorial.currentState == TutorialStateDone) {
        textBoxHide(&gTutorial.textBox);
        return;
    }

    if (!textBoxIsVisible(&gTutorial.textBox)) {
        if (gTutorial.nextState != gTutorial.currentState) {
            gTutorial.currentState = gTutorial.nextState;
        }

        gTutorial.waitForActionTimer -= gTimeDelta;

        if (gTutorial.waitForActionTimer <= 0.0f) {
            char* message = gTutorialStep[gTutorial.currentState].message;
            textBoxInit(&gTutorial.textBox, message, 230, SCREEN_WD >> 1, TUTORIAL_Y);
            textBoxSetIcon(&gTutorial.textBox, tutorialIconRenderer, &level);
        }
    }

    textBoxUpdate(&gTutorial.textBox);

    enum TutorialState nextState = gTutorial.nextState;
    float nextStateTime = WAIT_FOR_ACTION_TIME;
    struct Vector3* playerPos = &level->players[0].transform.position;

    switch (gTutorial.currentState) {
        case TutorialStateMove:
            if (vector3MagSqrd(&input->lastWorldDirection) > 0.001f) {
                nextState = TutorialStateJump;
            }
            break;
        case TutorialStateJump:
            if (playerInputGetDown(input, PlayerInputActionsJump)) {
                nextState = TutorialStateAttack;
            }
            break;
            break;
        case TutorialStateAttack:
            if (playerInputGetDown(input, PlayerInputActionsAttack)) {
                nextState = TutorialStateCapture;
            }
            break;
        case TutorialStateCapture:
            {
                struct LevelBase* base = tutorialGetBaseForTeam(level, playerPos, TEAM(0));
                if (base) {
                    level->baseCommandMenu[0].flags |= BaseCommandMenuFlagsForceHideOpenCommand;
                    nextState = TutorialStateBaseMenu;
                    nextStateTime = 0.0f;
                    break;
                }
            }
            break;
        case TutorialStateBaseMenu:
            {
                if (level->baseCommandMenu[0].flags & BaseCommandMenuFlagsShowingMenu) {
                    textBoxHide(&gTutorial.textBox);
                } else {
                    gTutorial.textBox.nextState = TextBoxStateShowing;
                }

                struct LevelBase* base = tutorialGetBaseForTeam(level, playerPos, TEAM(0));

                if (base->defaultComand != MinionCommandDefend) {
                    nextState = TutorialStateCollectMinions;
                }
            }
            break;
        case TutorialStateCollectMinions:
            {
                level->baseCommandMenu[0].flags &= ~BaseCommandMenuFlagsForceHideOpenCommand;
                struct Minion* minion = tutorialGetMinion(level, TEAM(0), MinionCommandFollow);

                if (minion && (input->actionFlags & PlayerInputActionsCommandRecall)) {
                    nextState = TutorialStateOrderMinions;
                }
            }
            break;
        case TutorialStateOrderMinions:
            {
                struct Minion* minion = tutorialGetMinion(level, TEAM(0), MinionCommandAttack);

                if (minion) {
                    nextState = TutorialStateWin;
                }

                minion = tutorialGetMinion(level, TEAM(0), MinionCommandDefend);

                if (minion) {
                    nextState = TutorialStateCollectMinions;
                }
            }
            break;
        case TutorialStateWin:
            {
                if (gLastDamageTime > 0.0f) {
                    gTutorial.nextState = TutorialStateDone;
                }
            }
            break;
        case TutorialStateDone:
            break;
    }

    if (nextState != gTutorial.nextState) {
        gTutorial.nextState = nextState;
        gTutorial.waitForActionTimer = nextStateTime;
        textBoxHide(&gTutorial.textBox);
    }
}

void tutorialRender(struct LevelScene* level, struct RenderState* renderState) {
    struct TutorialStep* step = &gTutorialStep[gTutorial.currentState];
        
    textBoxRender(&gTutorial.textBox, renderState);

    if (gTutorial.currentState == TutorialStateDone || !textBoxIsVisible(&gTutorial.textBox)) {
        return;
    }

    if (step->flags) {
        struct Vector3* target = 0;
        struct Vector3* playerPos = &level->players[0].transform.position;
        if (step->flags & TutorailStepFlagsMoveToBase) {
            struct LevelBase* base = tutorialGetBaseForTeam(level, playerPos, step->team);
            if (base) {
                float distance = vector3DistSqrd(&base->position, playerPos);
                if (distance > SCENE_SCALE * SCENE_SCALE) {
                    target = &base->position;
                }
            }
        } else if (step->flags & TutorailStepFlagsMoveToMinion) {
            struct Minion* minion = tutorialGetMinion(level, step->team, -1);
            if (minion) {
                target = &minion->transform.position;
            }
        }

        if (target && mathfMod(gTimePassed, ARROW_FLICKER_TIME) > ARROW_FLICKER_TIME * 0.5f) {
            struct Transform transform;
            struct Vector3 offset;
            vector3Sub(target, playerPos, &offset);
            quatLook(&offset, &gUp, &transform.rotation);
            vector3AddScaled(playerPos, &gUp, SCENE_SCALE, &transform.position);
            quatMultVector(&transform.rotation, &gForward, &offset);
            vector3AddScaled(&transform.position, &offset, - 2.0f * SCENE_SCALE, &transform.position);
            transform.scale = gOneVec;

            Mtx* matrix = renderStateRequestMatrices(renderState, 1);
            transformToMatrixL(&transform, matrix);
            gSPMatrix(renderState->transparentDL++, matrix, G_MTX_MUL | G_MTX_MODELVIEW | G_MTX_PUSH);
            gSPDisplayList(renderState->transparentDL++, ObjectiveArrow_mesh);
            gSPPopMatrix(renderState->transparentDL++, G_MTX_MODELVIEW);
        }
    }
}