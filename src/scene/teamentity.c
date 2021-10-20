#include "teamentity.h"
#include "minion.h"
#include "player.h"
#include "levelbase.h"

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
    struct Vector3* aPos = teamEntityGetPosition((struct TeamEntity*)overlap->thisEntry->data);
    struct Vector3* bPos = (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) ? teamEntityGetPosition((struct TeamEntity*)overlap->otherEntry->data) : 0;

    struct Vector3 overlap3D;
    overlap3D.x = overlap->shapeOverlap.normal.x;
    overlap3D.y = 0.0f;
    overlap3D.z = overlap->shapeOverlap.normal.y;

    if (bPos) {
        vector3AddScaled(aPos, &overlap3D, -0.5f * overlap->shapeOverlap.depth, aPos);
        vector3AddScaled(bPos, &overlap3D, 0.5f * overlap->shapeOverlap.depth, bPos);
    } else {
        vector3AddScaled(aPos, &overlap3D, -overlap->shapeOverlap.depth, aPos);
    }
}