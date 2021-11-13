
#include "tutorial.h"

#include "graphics/gfx.h"
#include "../data/gameplaymenu/menu.h"
#include "graphics/sprite.h"
#include "util/time.h"

#define TUTORIAL_Y 190

#define WAIT_FOR_ACTION_TIME 2.0f
#define ARROW_FLICKER_TIME  0.5f

struct Tutorial gTutorial;

struct TutorialStep gTutorialStep[] = {
    {
        "Move",
        {16, 48, 16, 16},
        0,
    },
    {
        "Jump",
        {0, 0, 16, 16},
        0,
    },
    {
        "Attack",
        {16, 0, 16, 16},
        0,
    },
    {
        "Capture Base",
        {0, 0, 0, 0},
        TutorailStepFlagsNeutralBase,
    },
};

int tutorialIconRenderer(struct RenderState* renderState, void* data, int x, int y) {
    struct SpriteTile* tile = &gTutorialStep[gTutorial.currentState].icon;

    if (tile->w == 0) {
        return 0;
    }

    if (renderState) {
        spriteCopyImage(
            renderState, 
            LAYER_IMAGE_COPIES, 
            gButtonsImage, 
            32, 
            64, 
            x, 
            y, 
            tile->w, 
            tile->h, 
            tile->x, 
            tile->y
        );
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
    float distance = 10000000.0f;

    for (unsigned i = 0; i < level->baseCount; ++i) {
        struct LevelBase* curr = &level->bases[i];

        if (curr->team.teamNumber != team && (team != TEAM_NONE || curr->state != LevelBaseStateNeutral)) {
            continue;
        }

        float currDistance = vector3DistSqrd(&curr->position, near);

        if (currDistance < distance) {
            distance = currDistance;
            result = curr;
        }
    }

    return result;
}

void tutorialUpdate(struct LevelScene* level, struct PlayerInput* input) {
    if (!textBoxIsVisible(&gTutorial.textBox)) {
        if (gTutorial.nextState != gTutorial.currentState) {
            gTutorial.currentState = gTutorial.nextState;
        }

        gTutorial.waitForActionTimer -= gTimeDelta;

        if (gTutorial.waitForActionTimer <= 0.0f) {
            char* message = gTutorialStep[gTutorial.currentState].message;
            textBoxInit(&gTutorial.textBox, message, 200, SCREEN_WD >> 1, TUTORIAL_Y);
            textBoxSetIcon(&gTutorial.textBox, tutorialIconRenderer, &level);
        }
    }

    textBoxUpdate(&gTutorial.textBox);

    enum TutorialState nextState = gTutorial.nextState;

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
            break;
        case TutorialStateCapture:
            break;
        case TutorialStateBaseMenu:
            break;
        case TutorialStateMinions:
            break;
        case TutorialStateWin:
            break;
    }

    if (nextState != gTutorial.nextState) {
        gTutorial.nextState = nextState;
        gTutorial.waitForActionTimer = WAIT_FOR_ACTION_TIME;
        textBoxHide(&gTutorial.textBox);
    }
}

void tutorialRender(struct LevelScene* level, struct RenderState* renderState) {
    struct TutorialStep* step = &gTutorialStep[gTutorial.currentState];
        
    textBoxRender(&gTutorial.textBox, renderState);

    if (step->flags) {
        struct Vector3* target = 0;
        struct Vector3* playerPos = &level->players[0].transform.position;
        if (step->flags & TutorailStepFlagsNeutralBase) {
            struct LevelBase* base = tutorialGetBaseForTeam(level, playerPos, TEAM_NONE);
            if (base) {
                if (gPlayerAtBase[0] == base) {
                    textBoxHide(&gTutorial.textBox);
                    gTutorial.waitForActionTimer = WAIT_FOR_ACTION_TIME;
                } else {
                    target = &base->position;
                }
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