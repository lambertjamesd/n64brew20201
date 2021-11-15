#ifndef _ITEM_DROP_H
#define _ITEM_DROP_H

#include "collision/dynamicscene.h"
#include "graphics/render_state.h"

#define MAX_ITEM_DROP       10

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
    float stateTimer;
    struct DynamicSceneEntry* collision;
};

void itemDropInit(struct ItemDrop* itemDrop);
void itemDropBegin(struct ItemDrop* itemDrop);
void itemDropUpdate(struct ItemDrop* itemDrop);
void itemDropRender(struct ItemDrop* itemDrop, struct RenderState* renderState);

struct ItemDrops {
    struct ItemDrop drops[MAX_ITEM_DROP];
    float nextDropTimer;
};

void itemDropsInit(struct ItemDrops* itemDrops);
void itemDropsUpdate(struct ItemDrops* itemDrops);
Gfx* itemDropsRender(struct ItemDrops* itemDrops, struct RenderState* renderState);

#endif