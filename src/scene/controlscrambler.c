#include "controlscrambler.h"
#include "util/memory.h"
#include "util/time.h"
#include "math/mathf.h"

#define CAMERA_ROTATE_DURATION 0.5f

float gDebuffTime[ControlsScramblerTypeCount] = {
    8.0f,
    8.0f,
    8.0f,
    8.0f,
    8.0f,
    8.0f,
    8.0f,
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

void controlsScramblerApply(struct ControlsScrambler* scrambler) {
    if (scrambler->timers[ControlsScramblerTypeJoystickFipped]) {
        vector3Negate(&scrambler->playerInput.targetWorldDirection, &scrambler->playerInput.targetWorldDirection);
    }

    if (scrambler->timers[ControlsScramblerTypeButtonsFlipped]) {
        scrambler->playerInput.actionFlags = controlsScramblerSwapFlags(scrambler->playerInput.actionFlags, PlayerInputActionsJump, PlayerInputActionsAttack);
        scrambler->playerInput.prevActions = controlsScramblerSwapFlags(scrambler->playerInput.prevActions, PlayerInputActionsJump, PlayerInputActionsAttack);
    }

    if (scrambler->timers[ControlsScramblerTypeAttackTurbo]) {
        int newFlagValue = (scrambler->playerInput.actionFlags ^ scrambler->playerInput.prevActions ^ PlayerInputActionsAttack) & PlayerInputActionsAttack;
        scrambler->playerInput.actionFlags = (scrambler->playerInput.actionFlags & ~PlayerInputActionsAttack) | newFlagValue;
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