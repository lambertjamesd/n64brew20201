#ifndef _SCENE_BASE_H
#define _SCENE_BASE_H

#include "math/vector2.h"
#include "collision/dynamicscene.h"
#include "graphics/render_state.h"
#include "factionentity.h"
#include "leveldefinition.h"

enum LevelBaseState {
    LevelBaseStateNeutral,
    LevelBaseStateSpawning,
    LevelBaseStateUpgradingSpawnRate,
    LevelBaseStateUpgradingCapacity,
    LevelBaseStateUpgradingDefence,
};

struct LevelBase {
    struct FactionEntity faction;
    struct Vector2 position;
    struct DynamicSceneEntry* collider;
    float stateTimeLeft;
    float captureProgress;
    unsigned char state;
    unsigned char minionCount;
    unsigned char baseId;
};

void levelBaseInit(struct LevelBase* base, struct BaseDefinition* definition, unsigned char baseId, unsigned int makeNeutral);
void levelBaseUpdate(struct LevelBase* base);
void levelBaseRender(struct LevelBase* base, struct RenderState* renderState);
void levelBaseReleaseMinion(struct LevelBase* base);

#endif