#ifndef _SCENE_BASE_H
#define _SCENE_BASE_H

#include "math/vector2.h"
#include "math/vector3.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"
#include "teamentity.h"
#include "leveldefinition.h"
#include "game_defs.h"

#define BASE_GFX_PER_BASE   11

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
    unsigned char state;
    unsigned char minionCount;
    unsigned char baseId;
    unsigned char speedUpgrade:2;
    unsigned char capacityUpgrade:2;
    unsigned char defenseUpgrade:2;
    unsigned char padding: 2;
    unsigned char defaultComand;
};

extern struct LevelBase* gPlayerAtBase[MAX_PLAYERS];

void levelBaseInit(struct LevelBase* base, struct BaseDefinition* definition, unsigned char baseId, unsigned int makeNeutral);
void levelBaseUpdate(struct LevelBase* base);
void levelBaseRender(struct LevelBase* base, struct RenderState* renderState);
void levelBaseReleaseMinion(struct LevelBase* base);
void levelBaseStartUpgrade(struct LevelBase* base, enum LevelBaseState nextState);

#endif