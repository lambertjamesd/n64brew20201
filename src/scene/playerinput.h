#ifndef _SCENE_PLAYER_INPUT_H
#define _SCENE_PLAYER_INPUT_H

#include <ultra64.h>
#include "math/vector2.h"
#include "math/quaternion.h"

enum PlayerInputActions {
    PlayerInputActionsJump = (1 << 0),
    PlayerInputActionsAttack = (1 << 0),
};

struct PlayerInput {
    struct Vector3 targetWorldDirection;
    unsigned short actionFlags;
    unsigned short prevActions;
};

void playerInputPopulateWithJoystickData(OSContPad* pad, unsigned short lastButtons, struct Quaternion* cameraRotation, struct PlayerInput* output);

#endif