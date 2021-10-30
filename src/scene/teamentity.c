#include "teamentity.h"
#include "minion.h"
#include "player.h"
#include "levelbase.h"
#include "util/time.h"
#include "events.h"

struct Vector3* teamEntityGetPosition(struct TeamEntity* entity) {
    if (!entity) {
        return 0;
    }

    switch (entity->entityType) {
        case TeamEntityTypeMinion:
            return &(((struct Minion*)entity)->transform.position);
        case TeamEntityTypePlayer:
            return &(((struct Player*)entity)->transform.position);
        case TeamEntityTypeBase:
            return &(((struct LevelBase*)entity)->position);
    }

    return 0;
}

void teamEntityCorrectOverlap(struct DynamicSceneOverlap* overlap) {
    struct TeamEntity* entityA = (struct TeamEntity*)overlap->thisEntry->data;
    struct TeamEntity* entityB = (struct TeamEntity*)overlap->otherEntry->data;
    struct Vector3* aPos = teamEntityGetPosition(entityA);
    struct Vector3* bPos = (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) ? teamEntityGetPosition(entityB) : 0;

    struct Vector3 overlap3D;
    overlap3D.x = overlap->shapeOverlap.normal.x;
    overlap3D.y = 0.0f;
    overlap3D.z = overlap->shapeOverlap.normal.y;

    if (bPos) {
        float weight = 0.5f;

        if (entityA->entityType != entityB->entityType) {
            if (entityA->entityType == TeamEntityTypePlayer) {
                weight = 0.1f;
            } else {
                weight = 0.9f;
            }
        }

        vector3AddScaled(aPos, &overlap3D, -weight * overlap->shapeOverlap.depth, aPos);
        vector3AddScaled(bPos, &overlap3D, (1.0f - weight) * overlap->shapeOverlap.depth, bPos);
    } else {
        vector3AddScaled(aPos, &overlap3D, -overlap->shapeOverlap.depth, aPos);
    }
}

void teamEntityApplyDamage(struct TeamEntity* entity, float amount) {
    gLastDamageTime = gTimePassed;

    switch (entity->entityType) {
        case TeamEntityTypeMinion:
            minionApplyDamage((struct Minion*)entity, amount);
            break;
        case TeamEntityTypePlayer:
            playerApplyDamage((struct Player*)entity, amount);
            break;
    }
}

int teamEntityIsAlive(struct TeamEntity* entity) {
    switch (entity->entityType) {
        case TeamEntityTypeMinion:
            return minionIsAlive((struct Minion*)entity);
        case TeamEntityTypePlayer:
            return playerIsAlive((struct Player*)entity);
        case TeamEntityTypeBase:
            return 1;
    }

    return 0;
}