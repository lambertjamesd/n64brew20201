#ifndef _SCENE_BASE_H
#define _SCENE_BASE_H

#include "math/vector2.h"
#include "math/vector3.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"
#include "teamentity.h"
#include "leveldefinition.h"
#include "game_defs.h"
#include "audio/soundplayer.h"

#define BASE_GFX_PER_BASE   12

enum LevelBaseState {
    LevelBaseStateNeutral,
    LevelBaseStateSpawning,
    LevelBaseStateUpgradingSpawnRate,
    LevelBaseStateUpgradingCapacity,
    LevelBaseStateUpgradingDefence,
};

struct LevelBase {
    struct TeamEntity team;
    struct Vector3 position;
    struct DynamicSceneEntry* collider;
    float stateTimeLeft;
    float captureProgress;
    float lastCaptureProgress;
    SoundID captureSound;
    unsigned char state;
    unsigned char minionCount;
    unsigned char baseId;
    unsigned char speedUpgrade:2;
    unsigned char capacityUpgrade:2;
    unsigned char defenseUpgrade:2;
    unsigned char padding: 2;
    unsigned char defaultComand;
    unsigned char issueCommandTimer;
    unsigned char followPlayer;
    unsigned char baseControlCount[MAX_PLAYERS];
    unsigned char prevControlCount[MAX_PLAYERS];
};

extern struct LevelBase* gPlayerAtBase[MAX_PLAYERS];

void levelBaseInit(struct LevelBase* base, struct BaseDefinition* definition, unsigned char baseId, unsigned int makeNeutral);
void levelBaseUpdate(struct LevelBase* base);
void levelBaseRender(struct LevelBase* base, struct RenderState* renderState);
void levelBaseReleaseMinion(struct LevelBase* base);
void levelBaseStartUpgrade(struct LevelBase* base, enum LevelBaseState nextState);
void levelBaseSetDefaultCommand(struct LevelBase* base, unsigned command, unsigned fromPlayer);
int levelBaseGetTeam(struct LevelBase* base);
int levelBaseIsBeingCaptured(struct LevelBase* base);

#endif