#ifndef _SCENE_PLAYER_H
#define _SCENE_PLAYER_H

#include "teamentity.h"
#include "math/transform.h"
#include "sk64/skelatool_animation.h"
#include "sk64/skelatool_armature.h"
#include "math/vector2.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"
#include "playerinput.h"

struct Player;

typedef void (*PlayerState)(struct Player* player, struct PlayerInput* input);

struct Player {
    struct TeamEntity team;
    struct Transform transform;
    struct Vector3 velocity;
    struct Vector2 rightDir;
    struct SKAnimator animator;
    struct SKArmature armature;
    struct DynamicSceneEntry* collider;
    PlayerState state;
};

void playerInit(struct Player* player, unsigned team, struct Vector2* at);
void playerUpdate(struct Player* player, struct PlayerInput* input);
void playerRender(struct Player* player, struct RenderState* renderState);

#endif