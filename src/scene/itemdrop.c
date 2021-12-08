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
#include "../data/models/characters.h"
#include "controlscrambler.h"
#include "team_data.h"
#include "audio/soundplayer.h"

#define DROP_TIME       4.0f
#define DROP_LIFETIME   60.0f
#define INTAGIBLE_TIME  0.5f
#define DAMAGE_AMOUNT   3.0f
#define DROP_COLLIDER_RADIUS    (SCENE_SCALE)
#define FALL_VELOCITY (SCENE_SCALE * 40.0f)
#define GFX_PER_DROP            4
#define GFX_PER_CHASER_DROP     3
#define ROTATION_FREQ   (0.5 * (2 * M_PI))
#define FLICKER_AWAY_PERIOD   0.5f
#define FLICKER_AWAY_TIME     5.0f
#define FAVOR_PLAYER_RADIUS     (30.0f * SCENE_SCALE)

struct CollisionCircle gItemDropCollider = {
    {CollisionShapeTypeCircle},
    DROP_COLLIDER_RADIUS,
};

void itemDropInit(struct ItemDrop* itemDrop) {
    itemDrop->state = ItemDropDisabled;
    itemDrop->stateTimer = 0.0f;
    itemDrop->collision = 0;
    itemDrop->soundId = SOUND_ID_NONE;
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
    soundPlayerStop(&itemDrop->soundId);
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
            struct Vector3 pos3D;
            pos3D.x = drop->collision->center.x;
            pos3D.y = 0.0f;
            pos3D.z = drop->collision->center.y;
            teamEntityApplyDamage(entity, DAMAGE_AMOUNT, &pos3D, 10.f);
            break;
        }
        case ItemDropStateWaiting:
        {
            struct TeamEntity* entity = teamEntityGetFromCollision(overlap->otherEntry);

            if (entity->entityType == TeamEntityTypePlayer && DROP_LIFETIME - drop->stateTimer > INTAGIBLE_TIME) {
                drop->state = ItemDropStateCollected;
                struct Vector3 pos3D;
                pos3D.x = drop->collision->center.x;
                pos3D.y = SCENE_SCALE;
                pos3D.z = drop->collision->center.y;
                itemActivateScrambler(&gCurrentLevel, &pos3D, randomInRange(0, ControlsScramblerTypeCount), entity->teamNumber);
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

int itemDropIsValidLocation(struct ItemDrop* itemDrop, int favorPlayer) {
    if (!staticSceneInInsideBoundary(
                &gCurrentLevel.definition->staticScene, 
                &itemDrop->collision->center, 
                DROP_COLLIDER_RADIUS)) {
        return 0;
    }

    struct Vector3 pos3D;
    pos3D.x = itemDrop->collision->center.x;
    pos3D.y = 0.0f;
    pos3D.z = itemDrop->collision->center.y;

    if (favorPlayer && 
        vector3DistSqrd(&pos3D, &gCurrentLevel.players[0].transform.position) >= FAVOR_PLAYER_RADIUS * FAVOR_PLAYER_RADIUS) {
         return 0;   
    }

    return 1;
}

void itemDropUpdate(struct ItemDrop* itemDrop, int favorPlayer) {
    switch (itemDrop->state) {
        case ItemDropStateFindingDrop:
        {
            if (itemDropIsValidLocation(itemDrop, favorPlayer)) {
                itemDrop->state = ItemDropStateCheckingDrop;
            } else {
                struct Vector2 newPos;
                itemDropRandomLocation(&newPos);
                dynamicEntrySetPos(itemDrop->collision, &newPos);
            }
            break;
        }
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
        {
            struct Vector3 pos3D;
            pos3D.x = itemDrop->collision->center.x;
            pos3D.y = 0.0f;
            pos3D.z = itemDrop->collision->center.y;
            soundPlayerPlay(SOUNDS_ITEMSPAWN, 1.0f, SoundPlayerPriorityNonPlayer, 0, &pos3D);
            itemDrop->soundId = soundPlayerPlay(SOUNDS_ITEMWAIT, 0.25f, SoundPlayerPriorityBackground, 0, &pos3D);
            itemDrop->state = ItemDropStateWaiting;
            break;
        }
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

        if (itemDrop->state == ItemDropStateFalling) {
            quatAxisAngle(&gUp, gTimePassed * M_PI * 4.0f, &transform.rotation);
        }

        transformToMatrixL(&transform, matrix);
        gSPMatrix(renderState->dl++, matrix, G_MTX_PUSH | G_MTX_MODELVIEW | G_MTX_MUL);
        gSPDisplayList(renderState->dl++, ItemPickup_ItemDropBase_mesh);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

        if (itemDrop->state == ItemDropStateFalling) {
            gSPMatrix(renderState->transparentDL++, matrix, G_MTX_PUSH | G_MTX_MODELVIEW | G_MTX_MUL);
            gSPDisplayList(renderState->transparentDL++, ItemPickup_Fireball_mesh);
            gSPPopMatrix(renderState->transparentDL++, G_MTX_MODELVIEW);
        }
    }

    if (itemDrop->state == ItemDropStateWaiting && (itemDrop->stateTimer > FLICKER_AWAY_TIME || mathfMod(gTimePassed, FLICKER_AWAY_PERIOD) < FLICKER_AWAY_PERIOD * 0.5f)) {
        Mtx* matrix = renderStateRequestMatrices(renderState, 2);

        if (!matrix) {
            return;
        }

        transform.position.y = 0.0f;
        quatAxisAngle(&gUp, gTimePassed * ROTATION_FREQ, &transform.rotation);
        transformToMatrixL(&transform, matrix);
        gSPMatrix(renderState->transparentDL++, matrix, G_MTX_PUSH | G_MTX_MODELVIEW | G_MTX_MUL);
        gSPDisplayList(renderState->transparentDL++, ItemPickup_ItemDrop_mesh);
        guScale(&matrix[1], 1.25f, 2.5f + (mathfMod(gTimePassed, 0.125f) * 1.0f), 1.25f);
        gSPMatrix(renderState->transparentDL++, &matrix[1], G_MTX_NOPUSH | G_MTX_MODELVIEW | G_MTX_MUL);
        gDPSetPrimColor(renderState->transparentDL++, 255, 255, 120, 240, 200, 160);
        gSPDisplayList(renderState->transparentDL++, RecallCircle_Cylinder_mesh);
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

void itemDropChaserInit(struct ItemDropChaser* chaser) {
    pathfinderReset(&chaser->pathfinder);
    chaser->scrambleType = ControlsScramblerTypeCount;
}

int itemDropChaserIsActive(struct ItemDropChaser* chaser) {
    return chaser->scrambleType != ControlsScramblerTypeCount;
}

void itemDropChaserActivate(struct ItemDropChaser* chaser, struct Vector3* from, enum ControlsScramblerType scramblerType, int index) {
    // if a new pickup happens before the current animation finishes
    // just apply the effect now and start the new animation
    if (itemDropChaserIsActive(chaser)) {
        levelSceneApplyScrambler(&gCurrentLevel, index, chaser->scrambleType);
    }

    chaser->scrambleType = scramblerType;
    punchTrailInit(&chaser->punchTrail, from, 0.5f);
    pathfinderSetTarget(&chaser->pathfinder, &gCurrentLevel.definition->pathfinding, from, &gCurrentLevel.players[index].transform.position);
}

void itemDropChaserUpdate(struct ItemDropChaser* chaser, int index) {
    if (itemDropChaserIsActive(chaser)) {
        pathfinderUpdate(
            &chaser->pathfinder, 
            &gCurrentLevel.definition->pathfinding,
            punchTrailHeadPosition(&chaser->punchTrail),
            0
        );

        struct Vector3 nextTarget;

        if (chaser->pathfinder.currentNode != NODE_NONE) {
            nextTarget = gCurrentLevel.definition->pathfinding.nodePositions[chaser->pathfinder.currentNode];
        } else {
            nextTarget = gCurrentLevel.players[index].transform.position;
        }

        nextTarget.y = SCENE_SCALE;

        struct Vector3 nextHeadPos;
        if (vector3MoveTowards(punchTrailHeadPosition(&chaser->punchTrail), &nextTarget, ITEM_CHASER_SPEED * gTimeDelta, &nextHeadPos) && 
            chaser->pathfinder.currentNode == NODE_NONE) {
            soundPlayerPlay(SOUNDS_CONTROLSCRAMBLE, 1.0f, SoundPlayerPriorityPlayer, 0, &nextHeadPos);
            levelSceneApplyScrambler(&gCurrentLevel, index, chaser->scrambleType);
            chaser->scrambleType = ControlsScramblerTypeCount;
        } else {
            nextHeadPos.x += randomInRangef(-SCENE_SCALE, SCENE_SCALE);
            nextHeadPos.z += randomInRangef(-SCENE_SCALE, SCENE_SCALE);

            punchTrailUpdate(&chaser->punchTrail, &nextHeadPos);
        }
    }
}

void itemDropChaserRender(struct ItemDropChaser* chaser, struct RenderState* renderState, unsigned index) {
    if (itemDropChaserIsActive(chaser)) {
        punchTrailRender(&chaser->punchTrail, renderState, gTeamColors[index]);
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

void itemDropsInit(struct ItemDrops* itemDrops, int favorPlayer) {
    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        itemDropInit(&itemDrops->drops[i]);
    }
    for (unsigned i = 0; i < MAX_PLAYERS; ++i) {
        itemDropChaserInit(&itemDrops->chasers[i]);
    }
    itemDrops->nextDropTimer = itemDropsNextTime(0);
    itemDrops->favorPlayer = favorPlayer;
}

void itemDropsUpdate(struct ItemDrops* itemDrops) {
    for (unsigned i = 0; i < gCurrentLevel.playerCount; ++i) {
        itemDropChaserUpdate(&itemDrops->chasers[i], i);
    }

    unsigned activeDrops = 0;
    struct ItemDrop* nextDrop = 0;
    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        itemDropUpdate(&itemDrops->drops[i], itemDrops->favorPlayer);

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
    Gfx* result = renderStateAllocateDLChunk(renderState, GFX_PER_DROP * MAX_ITEM_DROP + MAX_PLAYERS * GFX_PER_CHASER_DROP + 2);
    Gfx* prevDL = renderStateReplaceDL(renderState, result);

    for (unsigned i = 0; i < gCurrentLevel.playerCount; ++i) {
        itemDropChaserRender(&itemDrops->chasers[i], renderState, i);
    }

    gSPDisplayList(renderState->dl++, mat_ItemPickup_Drop_Platform);

    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        itemDropRender(&itemDrops->drops[i], renderState);
    }

    gSPEndDisplayList(renderState->dl++);
    Gfx* resultEnd = renderStateReplaceDL(renderState, prevDL);
    assert(resultEnd <= result + GFX_PER_DROP * MAX_ITEM_DROP + 1);

    return result;
}


void itemActivateScrambler(struct LevelScene* scene, struct Vector3* from, enum ControlsScramblerType scramblerType, int fromTeam) {
    soundPlayerPlay(SOUNDS_POWERUP_PICKUP, 1.0f, SoundPlayerPriorityNonPlayer, 0, from);
    for (unsigned i = 0; i < scene->playerCount; ++i) {
        if (i != fromTeam) {
            itemDropChaserActivate(&scene->itemDrops.chasers[i], from, scramblerType, i);
        }
    }
}

struct ItemDrop* itemDropsClosest(struct ItemDrops* itemDrops, struct Vector3* to, float maxDistance) {
    struct ItemDrop* result = 0;
    float distance = maxDistance * maxDistance;

    struct Vector2 posAs2D;
    posAs2D.x = to->x;
    posAs2D.y = to->z;

    for (unsigned i = 0; i < MAX_ITEM_DROP; ++i) {
        if (itemDrops->drops[i].state == ItemDropStateWaiting) {
            float itemDistance = vector2DistSqr(&posAs2D, &itemDrops->drops[i].collision->center);

            if (itemDistance < distance) {
                result = &itemDrops->drops[i];
                distance = itemDistance;
            }
        }
    }

    return result;
}