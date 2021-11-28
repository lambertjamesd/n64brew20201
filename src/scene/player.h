#ifndef _SCENE_PLAYER_H
#define _SCENE_PLAYER_H

#include "teamentity.h"
#include "levelbase.h"
#include "math/transform.h"
#include "sk64/skelatool_animator.h"
#include "sk64/skelatool_armature.h"
#include "math/vector2.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"
#include "playerinput.h"
#include "recall_circle.h"
#include "punchtrail.h"
#include "audio/soundplayer.h"
#include "damagehandler.h"
#include "faction.h"

#define PLAYER_GFX_PER_PLAYER   8

#define PLAYER_DEFENSE_RADIUS  10.0f

#define PLAYER_COLLIDER_RADIOUS (0.5f * SCENE_SCALE)

struct Player;

typedef void (*PlayerState)(struct Player* player, struct PlayerInput* input);

enum PlayerFlags {
    PlayerFlagsInAttackWindow = (1 << 0),
    PlayerFlagsAttackEarly = (1 << 1),
};

struct Player {
    struct TeamEntity team;
    struct Transform transform;
    struct Vector3 velocity;
    struct Vector2 rightDir;
    struct SKAnimator animator;
    struct SKArmature armature;
    struct DynamicSceneEntry* collider;
    struct RecallCircle recallCircle;
    struct DynamicSceneEntry* attackCollider;
    struct PlayerAttackInfo* attackInfo;
    struct PunchTrail punchTrail;
    PlayerState state;
    unsigned short playerIndex;
    unsigned short flags;
    unsigned short controlledBases;
    SoundID walkSoundEffect;
    SoundID idleSoundEffect;
    struct DamageHandler damageHandler;
    float stateTimer;
    float animationSpeed;
    struct TeamEntity *touchedBy;
    struct TeamEntity *aiTarget;
};

int aiAttackPriority(struct TeamEntity* target);

void playerInit(struct Player* player, unsigned playerIndex, unsigned team, struct Vector2* at);
void playerUpdate(struct Player* player, struct PlayerInput* input);
void playerRender(struct Player* player, struct RenderState* renderState);
void playerApplyDamage(struct Player* player, float amount);
int playerIsAlive(struct Player* player);

#endif