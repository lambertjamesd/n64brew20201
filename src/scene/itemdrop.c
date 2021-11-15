#include "itemdrop.h"
#include "collision/circle.h"
#include "game_defs.h"
#include "scene_management.h"
#include "math/mathf.h"
#include "collision/collisionlayers.h"
#include "util/time.h"
#include "assert.h"
#include "../data/models/itemdrop/header.h"
#include "../data/models/itemdropbase/header.h"
#include "../data/models/target/header.h"
#include "controlscrambler.h"

#define DROP_TIME       4.0f
#define DROP_LIFETIME   60.0f
#define DAMAGE_AMOUNT   3.0f
#define DROP_COLLIDER_RADIUS    (SCENE_SCALE)
#define FALL_VELOCITY (SCENE_SCALE * 40.0f)
#define GFX_PER_DROP    4
#define ROTATION_FREQ   (0.5 * (2 * M_PI))
#define FLICKER_AWAY_PERIOD   0.5f
#define FLICKER_AWAY_TIME     5.0f

struct CollisionCircle gItemDropCollider = {
    {CollisionShapeTypeCircle},
    DROP_COLLIDER_RADIUS,
};

void itemDropInit(struct ItemDrop* itemDrop) {
    itemDrop->state = ItemDropDisabled;
    itemDrop->stateTimer = 0.0f;
    itemDrop->collision = 0;
}

void itemDropRandomLocation(struct Vector2* output) {
    output->x = randomInRangef(
        gCurrentLevel.definition->levelBoundaries.min.x,
        gCurrentLevel.definition->levelBoundaries.max.x
    );
    output->y = randomInRangef(
        gCurrentLevel.definition->levelBoundaries.min.y,
        gCurrentLevel.definition->levelBoundaries.max.y
    );
}

void itemDropCleanup(struct ItemDrop* itemDrop) {
    dynamicSceneDeleteEntry(itemDrop->collision);
    itemDrop->collision = 0;
    itemDrop->state = ItemDropDisabled;
}

void itemDropCollide(struct DynamicSceneOverlap* overlap) {
    struct ItemDrop* drop = (struct ItemDrop*)overlap->thisEntry->data;
    
    switch (drop->state) {
        case ItemDropStateCheckingDrop:
        {
            drop->state = ItemDropStateFindingDrop;
            struct Vector2 newPos;
            itemDropRandomLocation(&newPos);
            dynamicEntrySetPos(overlap->thisEntry, &newPos);
            break;
        }
        case ItemDropDamaging:
        {
            struct TeamEntity* entity = teamEntityGetFromCollision(overlap->otherEntry);
            teamEntityApplyDamage(entity, DAMAGE_AMOUNT);
            break;
        }
        case ItemDropStateWaiting:
        {
            struct TeamEntity* entity = teamEntityGetFromCollision(overlap->otherEntry);

            if (entity->entityType == TeamEntityTypePlayer) {
                drop->state = ItemDropStateCollected;
                itemDropCleanup(drop);
                levelSceneApplyScrambler(&gCurrentLevel, entity->teamNumber, randomInRange(0, ControlsScramblerTypeCount));
            }

            break;
        }
    }
}

void itemDropBegin(struct ItemDrop* itemDrop) {
    if (itemDrop->state == ItemDropDisabled) {
        struct Vector2 center;
        itemDropRandomLocation(&center);

        itemDrop->state = ItemDropStateFindingDrop;
        itemDrop->collision = dynamicSceneNewEntry(
            &gItemDropCollider.shapeCommon, 
            itemDrop,
            &center,
            itemDropCollide,
            0,
            CollisionLayersStatic
        );
    }
}

void itemDropUpdate(struct ItemDrop* itemDrop) {
    switch (itemDrop->state) {
        case ItemDropStateFindingDrop:
            if (staticSceneInInsideBoundary(
                &gCurrentLevel.definition->staticScene, 
                &itemDrop->collision->center, 
                DROP_COLLIDER_RADIUS)) {
                itemDrop->state = ItemDropStateCheckingDrop;
            } else {
                struct Vector2 newPos;
                itemDropRandomLocation(&newPos);
                dynamicEntrySetPos(itemDrop->collision, &newPos);
            }
            break;
        case ItemDropStateCheckingDrop:
            itemDrop->state = ItemDropStateFalling;
            itemDrop->stateTimer = DROP_TIME;
            break;
        case ItemDropStateFalling:
            itemDrop->stateTimer -= gTimeDelta;
            if (itemDrop->stateTimer < 0.0f) {
                itemDrop->state = ItemDropDamaging;
                itemDrop->stateTimer = DROP_LIFETIME;
                itemDrop->collision->flags |= DynamicSceneEntryIsTrigger;
                itemDrop->collision->collisionLayers = CollisionLayersTangible;
            }
            break;
        case ItemDropDamaging:
            itemDrop->state = ItemDropStateWaiting;
            break;
        case ItemDropStateWaiting:
            itemDrop->stateTimer -= gTimeDelta;
            if (itemDrop->stateTimer < 0.0f) {
                itemDropCleanup(itemDrop);
            }
            break;
        case ItemDropStateCollected:
            itemDropCleanup(itemDrop);
            break;
        default:
            break;
    }
}

void itemDropRender(struct ItemDrop* itemDrop, struct RenderState* renderState) {
    if (itemDrop->state == ItemDropDisabled || !itemDrop->collision) {
        return;
    }

    struct Transform transform;
    transform.position.x = itemDrop->collision->center.x;
    transform.position.y = 0.0f;
    transform.position.z = itemDrop->collision->center.y;
    quatIdent(&transform.rotation);
    transform.scale = gOneVec;

    if (itemDrop->state == ItemDropStateFalling || itemDrop->state == ItemDropStateWaiting) {
        Mtx* matrix = renderStateRequestMatrices(renderState, 1);

        if (!matrix) {
            return;
        }

        if (itemDrop->state == ItemDropStateFalling) {
            transform.position.y = FALL_VELOCITY * itemDrop->stateTimer;
        }

        transformToMatrixL(&transform, matrix);
        gSPMatrix(renderState->dl++, matrix, G_MTX_PUSH | G_MTX_MODELVIEW | G_MTX_MUL);
        gSPDisplayList(renderState->dl++, ItemPickup_ItemDropBase_mesh);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }

    if (itemDrop->state == ItemDropStateWaiting && (itemDrop->stateTimer > FLICKER_AWAY_TIME || mathfMod(gTimePassed, FLICKER_AWAY_PERIOD) < FLICKER_AWAY_PERIOD * 0.5f)) {
        Mtx* matrix = renderStateRequestMatrices(renderState, 1);

        if (!matrix) {
            return;
        }

        transform.position.y = 0.0f;
        quatAxisAngle(&gUp, gTimePassed * ROTATION_FREQ, &transform.rotation);
        transformToMatrixL(&transform, matrix);
        gSPMatrix(renderState->transparentDL++, matrix, G_MTX_PUSH | G_MTX_MODELVIEW | G_MTX_MUL);
        gSPDisplayList(renderState->transparentDL++, ItemPickup_ItemDrop_mesh);
        gSPPopMatrix(renderState->transparentDL++, G_MTX_MODELVIEW);
    } else if (itemDrop->state == ItemDropStateFalling) {
        Mtx* matrix = renderStateRequestMatrices(renderState, 1);

        if (!matrix) {
            return;
        }

        transform.position.y = 5.0f;
        quatAxisAngle(&gUp, gTimePassed * ROTATION_FREQ * 2.0f, &transform.rotation);
        transformToMatrixL(&transform, matrix);
        gSPMatrix(renderState->transparentDL++, matrix, G_MTX_PUSH | G_MTX_MODELVIEW | G_MTX_MUL);
        gSPDisplayList(renderState->transparentDL++, ItemPickup_Target_mesh);
        gSPPopMatrix(renderState->transparentDL++, G_MTX_MODELVIEW);
    }
}

float gNextDropTime[] = {
    5.0f,
    10.0f,
    15.0f,
    20.0f,
    25.0f,
    30.0f,
    35.0f,
    40.0f,
    45.0f,
    50.0f,
    55.0f,
};

float itemDropsNextTime(unsigned currentDropCount) {
    if (currentDropCount + 2 >= sizeof(gNextDropTime) / sizeof(*gNextDropTime)) {
        currentDropCount = sizeof(gNextDropTime) / sizeof(*gNextDropTime) - 2;
    }

    return randomInRangef(gNextDropTime[currentDropCount], gNextDropTime[currentDropCount+1]);
};

void itemDropsInit(struct ItemDrops* itemDrops) {
    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        itemDropInit(&itemDrops->drops[i]);
    }
    itemDrops->nextDropTimer = itemDropsNextTime(0);
}

void itemDropsUpdate(struct ItemDrops* itemDrops) {
    unsigned activeDrops = 0;
    struct ItemDrop* nextDrop = 0;
    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        itemDropUpdate(&itemDrops->drops[i]);

        if (itemDrops->drops[i].state != ItemDropDisabled) {
            ++activeDrops;
        } else {
            nextDrop = &itemDrops->drops[i];
        }
    }

    itemDrops->nextDropTimer -= gTimeDelta;

    if (itemDrops->nextDropTimer <= 0.0f) {
        if (nextDrop) {
            itemDropBegin(nextDrop);
            itemDrops->nextDropTimer = itemDropsNextTime(activeDrops);
        }
    }
}

Gfx* itemDropsRender(struct ItemDrops* itemDrops, struct RenderState* renderState) {
    Gfx* result = renderStateAllocateDLChunk(renderState, GFX_PER_DROP * MAX_ITEM_DROP);
    Gfx* prevDL = renderStateReplaceDL(renderState, result);

    gSPDisplayList(renderState->dl++, mat_ItemPickup_Drop_Platform);

    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        itemDropRender(&itemDrops->drops[i], renderState);
    }

    gSPEndDisplayList(renderState->dl++);
    Gfx* resultEnd = renderStateReplaceDL(renderState, prevDL);
    assert(resultEnd <= result + GFX_PER_DROP * MAX_ITEM_DROP + 1);

    return result;
}