#ifndef _ITEM_DROP_H
#define _ITEM_DROP_H

#include "collision/dynamicscene.h"
#include "graphics/render_state.h"
#include "punchtrail.h"
#include "ai/ai_pathfinder.h"
#include "game_defs.h"
#include "controlscrambler.h"
#include "audio/soundplayer.h"

#define MAX_ITEM_DROP       10

#define ITEM_CHASER_SPEED  (80.0f * SCENE_SCALE)

enum ItemDropState {
    ItemDropDisabled,
    ItemDropStateFindingDrop,
    ItemDropStateCheckingDrop,
    ItemDropStateFalling,
    ItemDropDamaging,
    ItemDropStateWaiting,
    ItemDropStateCollected,
};

struct ItemDrop {
    unsigned short state;
    SoundID soundId;
    float stateTimer;
    struct DynamicSceneEntry* collision;
};

void itemDropInit(struct ItemDrop* itemDrop);
void itemDropBegin(struct ItemDrop* itemDrop);
void itemDropUpdate(struct ItemDrop* itemDrop, int favorPlayer);
void itemDropRender(struct ItemDrop* itemDrop, struct RenderState* renderState);

struct ItemDropChaser {
    enum ControlsScramblerType scrambleType;
    struct PunchTrail punchTrail;
    struct Pathfinder pathfinder;
};

void itemDropChaserInit(struct ItemDropChaser* chaser);
int itemDropChaserIsActive(struct ItemDropChaser* chaser);
void itemDropChaserActivate(struct ItemDropChaser* chaser, struct Vector3* from, enum ControlsScramblerType scramblerType, int index);
void itemDropChaserUpdate(struct ItemDropChaser* chaser, int index);
void itemDropChaserRender(struct ItemDropChaser* chaser, struct RenderState* renderState, unsigned index);

struct ItemDrops {
    struct ItemDrop drops[MAX_ITEM_DROP];
    struct ItemDropChaser chasers[MAX_PLAYERS];
    float nextDropTimer;
    int favorPlayer;
};

void itemDropsInit(struct ItemDrops* itemDrops, int favorPlayer);
void itemDropsUpdate(struct ItemDrops* itemDrops);
Gfx* itemDropsRender(struct ItemDrops* itemDrops, struct RenderState* renderState);

struct ItemDrop* itemDropsClosest(struct ItemDrops* itemDrops, struct Vector3* to, float maxDistance);


struct LevelScene;

void itemActivateScrambler(struct LevelScene* scene, struct Vector3* from, enum ControlsScramblerType scramblerType, int fromTeam);

#endif