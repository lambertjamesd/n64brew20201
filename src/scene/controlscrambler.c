#include "controlscrambler.h"
#include "util/memory.h"
#include "util/time.h"
#include "math/mathf.h"
#include "scene/player.h"
#include "../data/models/characters.h"

#define CAMERA_ROTATE_DURATION 0.5f

float gDebuffTime[ControlsScramblerTypeCount] = {
    5.0f,
    6.0f,
    4.0f,
    7.0f,
    6.0f,
};

void controlsScramblerInit(struct ControlsScrambler* scrambler) {
    zeroMemory(scrambler, sizeof(struct ControlsScrambler));
}

void controlsScramblerUpdate(struct ControlsScrambler* scrambler) {
    for (unsigned i = 0; i < ControlsScramblerTypeCount; ++i) {
        if (scrambler->timers[i] > 0.0f) {
            scrambler->timers[i] -= gTimeDelta;

            if (scrambler->timers[i] < 0.0f) {
                scrambler->timers[i] = 0.0f;
            }
        }
    }
    scrambler->playerInput.prevActions = scrambler->playerInput.actionFlags;
    scrambler->playerInput.lastWorldDirection = scrambler->playerInput.targetWorldDirection;
}

unsigned controlsScramblerSwapFlags(unsigned input, unsigned a, unsigned b) {
    unsigned hasA = input & a;
    unsigned hasB = input & b;

    input &= ~(a | b);

    if (hasA) {
        input |= b;
    }

    if (hasB) {
        input |= a;
    }

    return input;
}

void controlsScramblerTrigger(struct ControlsScrambler* scrambler, enum ControlsScramblerType type) {
    if (type == ControlsScramblerViewFlipped && scrambler->timers[type]) {
        // special case to not repeat the flip animation
        scrambler->timers[type] = gDebuffTime[type] - CAMERA_ROTATE_DURATION;
    } else {
        scrambler->timers[type] = gDebuffTime[type];
    }
}

void controlsScramblerApply(struct ControlsScrambler* scrambler, int isAI) {
    if (scrambler->timers[ControlsScramblerTypeJoystickFipped]) {
        vector3Negate(&scrambler->playerInput.targetWorldDirection, &scrambler->playerInput.targetWorldDirection);
    }

    if (scrambler->timers[ControlsScramblerTypeButtonsFlipped]) {
        scrambler->playerInput.actionFlags = controlsScramblerSwapFlags(scrambler->playerInput.actionFlags, PlayerInputActionsJump, PlayerInputActionsAttack);
        scrambler->playerInput.prevActions = controlsScramblerSwapFlags(scrambler->playerInput.prevActions, PlayerInputActionsJump, PlayerInputActionsAttack);
    }

    if (scrambler->timers[ControlsScramblerTypeJumpTurbo]) {
        int newFlagValue = (scrambler->playerInput.actionFlags ^ scrambler->playerInput.prevActions ^ PlayerInputActionsJump) & PlayerInputActionsJump;
        scrambler->playerInput.actionFlags = (scrambler->playerInput.actionFlags & ~PlayerInputActionsJump) | newFlagValue;
    }

    if (scrambler->timers[ControlsScramblerTypeMoveTurbo]) {
        if (vector3MagSqrd(&scrambler->playerInput.targetWorldDirection) > 0.001f) {
            vector3Normalize(&scrambler->playerInput.targetWorldDirection, &scrambler->playerInput.targetWorldDirection);
        } else if (vector3MagSqrd(&scrambler->playerInput.lastWorldDirection) > 0.001f) {
            vector3Normalize(&scrambler->playerInput.lastWorldDirection, &scrambler->playerInput.targetWorldDirection);
        } else {
            scrambler->playerInput.targetWorldDirection.x = randomInRangef(-1.0f, 1.0f);
            scrambler->playerInput.targetWorldDirection.z = randomInRangef(-1.0f, 1.0f);
            vector3Normalize(&scrambler->playerInput.targetWorldDirection, &scrambler->playerInput.targetWorldDirection);
        }
    }

    if (isAI && scrambler->timers[ControlsScramblerViewFlipped]) {
        scrambler->playerInput.targetWorldDirection.x = -scrambler->playerInput.targetWorldDirection.x;
    }
}

unsigned controlsScramblerActiveCount(struct ControlsScrambler* scrambler) {
    unsigned result = 0;
    for (unsigned i = 0; i < ControlsScramblerTypeCount; ++i) {
        if (scrambler->timers[i] > 0.0f) {
            ++result;
        }
    }

    return result;
}

#define ORBIT_RADIUS    (SCENE_SCALE * 0.75f)
#define ORBIT_PERIOD    (2.0f * M_PI / 0.75f)

void controlsScramblerRender(struct ControlsScrambler* scrambler, struct Player* forPlayer, struct RenderState* renderState) {
    if (!playerIsAlive(forPlayer)) {
        return;
    }

    int count = controlsScramblerActiveCount(scrambler);

    if (count == 0) {
        return;
    }

    Mtx* matrix = renderStateRequestMatrices(renderState, count);

    if (!matrix) {
        return;
    }

    for (unsigned i = 0; i < ControlsScramblerTypeCount; ++i) {
        if (scrambler->timers[i] <= 0.0f) {
            continue;
        }

        struct Transform transform;
        vector3AddScaled(&forPlayer->transform.position, &gUp, 3.0f * SCENE_SCALE, &transform.position);
        float angle = gTimePassed * ORBIT_PERIOD + i * M_PI * 2.0f / ControlsScramblerTypeCount;
        vector3AddScaled(&transform.position, &gRight, sinf(angle) * ORBIT_RADIUS, &transform.position);
        vector3AddScaled(&transform.position, &gForward, cosf(angle) * ORBIT_RADIUS, &transform.position);
        transform.rotation = *renderState->cameraRotation;
        vector3Scale(&gOneVec, &transform.scale, 0.5f);
        transformToMatrixL(&transform, matrix);
        gSPMatrix(renderState->dl++, matrix, G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        gDPSetTileSize(renderState->dl++, 0, 320 - i * 80, 0, 380, 60);
        gSPDisplayList(renderState->dl++, Scramblers_Plane_mesh_tri_0);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
        ++matrix;
    }
}

int controlsScramblerIsActive(struct ControlsScrambler* scrambler, enum ControlsScramblerType type) {
    return scrambler->timers[type] > 0.0f;
}

float controlsScramblerGetCameraRotation(struct ControlsScrambler* scrambler) {
    float flipTimeLeft = scrambler->timers[ControlsScramblerViewFlipped];

    if (!flipTimeLeft) {
        return 0.0f;
    }

    if (flipTimeLeft < CAMERA_ROTATE_DURATION) {
        return flipTimeLeft * (M_PI / CAMERA_ROTATE_DURATION);
    }

    if (flipTimeLeft > gDebuffTime[ControlsScramblerViewFlipped] - CAMERA_ROTATE_DURATION) {
        return (flipTimeLeft - gDebuffTime[ControlsScramblerViewFlipped]) * (M_PI / CAMERA_ROTATE_DURATION);
    }

    return M_PI;
}