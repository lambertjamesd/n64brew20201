
#include "levelbase.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "../data/models/characters.h"
#include "util/time.h"
#include "level_scene.h"
#include "scene_management.h"

#include "game_defs.h"

#define CAPTURE_TIME     10
#define SPAWN_TIME       5

struct Vector3 gSpawnOffset[MAX_MINIONS_PER_BASE] = {
    {0.0f, 0.0f, SCENE_SCALE * 0.5f},
    {SCENE_SCALE * 0.5 * 0.866, 0.0f, -SCENE_SCALE * 0.5f * 0.5f},
    {-SCENE_SCALE * 0.5 * 0.866, 0.0f, -SCENE_SCALE * 0.5f * 0.5f},
};

struct CollisionCircle gBaseCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE,
};

void levelBaseSetState(struct LevelBase* base, enum LevelBaseState state) {
    if (base->state == state) {
        return;
    }

    base->state = state;

    switch (state) {
        case LevelBaseStateSpawning:
            base->stateTimeLeft = SPAWN_TIME;
            break;
    }
}

void levelBaseTrigger(struct DynamicSceneOverlap* overlap) {
    if (overlap->otherEntry->flags & DynamicSceneEntryHasFaction) {
        struct LevelBase* base = (struct LevelBase*)overlap->thisEntry->data;

        struct FactionEntity* teamEntity = (struct FactionEntity*)overlap->otherEntry->data;

        if (base->team.entityFaction == TEAM_NONE) {
            base->team.entityFaction = teamEntity->entityType;
        } 
        
        if (teamEntity->entityFaction != base->team.entityFaction) {
            base->captureProgress -= gTimeDelta;

            if (base->captureProgress <= 0.0f) {
                base->captureProgress = 0.0f;
                base->team.entityFaction = TEAM_NONE;
                base->state = LevelBaseStateNeutral;
            }
        } else {
            base->captureProgress += gTimeDelta;

            if (base->captureProgress >= CAPTURE_TIME) {
                base->captureProgress = CAPTURE_TIME;

                if (base->state == LevelBaseStateNeutral) {
                    base->state = LevelBaseStateSpawning;
                    base->stateTimeLeft = SPAWN_TIME;
                }
            }
        }
    }
}

void levelBaseInit(struct LevelBase* base, struct BaseDefinition* definition, unsigned char baseId, unsigned int makeNeutral) {
    base->team.entityType = FactionEntityTypeBase;
    base->team.entityFaction = makeNeutral ? TEAM_NONE : definition->startingFaction;
    base->position = definition->position;
    base->baseId = baseId;

    base->state = LevelBaseStateNeutral;

    if (base->team.entityFaction != TEAM_NONE) {
        levelBaseSetState(base, LevelBaseStateSpawning);
    }
    
    base->captureProgress = 0.0f;

    base->collider = dynamicSceneNewEntry(
        &gBaseCollider.shapeCommon, 
        base, 
        &definition->position,
        levelBaseTrigger,
        DynamicSceneEntryIsTrigger | DynamicSceneEntryHasFaction,
        CollisionLayersBase
    );
}

void levelBaseUpdate(struct LevelBase* base) {
    switch (base->state) {
        case LevelBaseStateSpawning:
            if (base->minionCount < MAX_MINIONS_PER_BASE) {
                base->stateTimeLeft -= gTimeDelta;

                if (base->stateTimeLeft <= 0) {
                    struct Transform minionTransform;
                    transformInitIdentity(&minionTransform);
                    minionTransform.position.x = base->position.x + gSpawnOffset[base->minionCount].x;
                    minionTransform.position.z = base->position.y + gSpawnOffset[base->minionCount].z;
                    levelSceneSpawnMinion(&gCurrentLevel, MinionTypeMelee, &minionTransform, base->baseId);
                    ++base->minionCount;

                    base->stateTimeLeft = SPAWN_TIME;
                }
                break;
            }
    };
}

void levelBaseRender(struct LevelBase* base, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    guTranslate(matrix, base->position.x, 0.0f, base->position.y);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, base_Cylinder_mesh);
    gSPPopMatrix(renderState->dl++, 1);
}

void levelBaseReleaseMinion(struct LevelBase* base) {
    --base->minionCount;
}