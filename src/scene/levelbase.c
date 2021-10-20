
#include "levelbase.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "../data/models/characters.h"
#include "util/time.h"
#include "level_scene.h"
#include "scene_management.h"
#include "team_data.h"
#include "math/mathf.h"

#include "game_defs.h"

#define CAPTURE_TIME        7
#define SPAWN_TIME          4
#define FLASHES_PER_SPAWN   5

float gSpawnTimeSpeedScalar[] = {
    1.0f,
    2.0f,
    3.0f,
};

float gSpawnTimeCaptureScalar[] = {
    1.0f,
    0.75f,
    0.5f,
};


float gCapacityScalar[] = {
    1.0f,
    1.4f,
    2.0f,
};

#define MAX_UPGRADE_COUNT       3

float gSpeedUpgradeTime[MAX_UPGRADE_COUNT] = {
    15.0f,
    18.0f,
    20.0f,
};

float gCapacityUpgradeTime[MAX_UPGRADE_COUNT] = {
    15.0f,
    18.0f,
    20.0f,
};

float gDefenseUpgradeTime[MAX_UPGRADE_COUNT] = {
    15.0f,
    18.0f,
    20.0f,
};

#define MIN_FLAG_HEIGHT     0.5f
#define MAX_FLAG_HEIGHT     5.0f

struct Vector3 gSpawnOffset[MAX_MINIONS_PER_BASE] = {
    {0.0f, 0.0f, SCENE_SCALE * 0.5f},
    {SCENE_SCALE * 0.5 * 0.866, 0.0f, -SCENE_SCALE * 0.5f * 0.5f},
    {-SCENE_SCALE * 0.5 * 0.866, 0.0f, -SCENE_SCALE * 0.5f * 0.5f},
};

struct Vector3 gFlagOffset = {2.0f * SCENE_SCALE, 0.0f, 2.0f * SCENE_SCALE};

struct CollisionCircle gBaseCollider = {
    {CollisionShapeTypeCircle},
    2.0f * SCENE_SCALE,
};

struct LevelBase* gPlayerAtBase[MAX_PLAYERS];

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
    if (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) {
        struct LevelBase* base = (struct LevelBase*)overlap->thisEntry->data;

        struct TeamEntity* teamEntity = (struct TeamEntity*)overlap->otherEntry->data;

        if (teamEntity->teamNumber == base->team.teamNumber && teamEntity->entityType == TeamEntityTypePlayer) {
            gPlayerAtBase[teamEntity->teamNumber] = base;
        }

        if (base->team.teamNumber == TEAM_NONE) {
            base->team.teamNumber = teamEntity->teamNumber;
        } 
        
        if (teamEntity->teamNumber != base->team.teamNumber) {
            base->captureProgress -= gTimeDelta * gSpawnTimeCaptureScalar[base->defenseUpgrade];

            if (base->captureProgress <= 0.0f) {
                base->captureProgress = 0.0f;
                base->team.teamNumber = TEAM_NONE;
                base->state = LevelBaseStateNeutral;
            }
        } else {
            base->captureProgress += gTimeDelta * gSpawnTimeCaptureScalar[base->defenseUpgrade];

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

void levelBaseStartUpgrade(struct LevelBase* base, enum LevelBaseState nextState) {
    if (base->state != LevelBaseStateSpawning) {
        return;
    }

    switch (nextState) {
        case LevelBaseStateUpgradingSpawnRate:
            if (base->speedUpgrade + 1 < MAX_UPGRADE_COUNT) {
                base->stateTimeLeft = gSpeedUpgradeTime[base->speedUpgrade];
                base->state = nextState;
            }
            break;
        case LevelBaseStateUpgradingCapacity:
            if (base->capacityUpgrade + 1 < MAX_UPGRADE_COUNT) {
                base->stateTimeLeft = gCapacityUpgradeTime[base->capacityUpgrade];
                base->state = nextState;
            }
            break;
        case LevelBaseStateUpgradingDefence:
            if (base->defenseUpgrade + 1 < MAX_UPGRADE_COUNT) {
                base->stateTimeLeft = gDefenseUpgradeTime[base->defenseUpgrade];
                base->state = nextState;
            }
            break;
    };
}

void levelBaseInit(struct LevelBase* base, struct BaseDefinition* definition, unsigned char baseId, unsigned int makeNeutral) {
    base->team.entityType = TeamEntityTypeBase;
    base->team.teamNumber = makeNeutral ? TEAM_NONE : definition->startingTeam;
    base->position.x = definition->position.x;
    base->position.z = definition->position.y;
    base->baseId = baseId;
    base->speedUpgrade = 0;
    base->capacityUpgrade = 0;
    base->defenseUpgrade = 0;
    base->padding = 0;
    base->state = LevelBaseStateNeutral;

    if (base->team.teamNumber != TEAM_NONE) {
        levelBaseSetState(base, LevelBaseStateSpawning);
    }
    
    if (makeNeutral) {
        base->captureProgress = 0.0f;
    } else {
        base->captureProgress = CAPTURE_TIME;
    }

    base->collider = dynamicSceneNewEntry(
        &gBaseCollider.shapeCommon, 
        base, 
        &definition->position,
        levelBaseTrigger,
        DynamicSceneEntryIsTrigger | DynamicSceneEntryHasTeam,
        CollisionLayersBase
    );
}

void levelBaseUpdate(struct LevelBase* base) {
    switch (base->state) {
        case LevelBaseStateSpawning:
            if (base->minionCount <= base->capacityUpgrade) {
                base->stateTimeLeft -= gTimeDelta * gSpawnTimeSpeedScalar[base->speedUpgrade];

                if (base->stateTimeLeft <= 0.0f) {
                    struct Transform minionTransform;
                    transformInitIdentity(&minionTransform);
                    vector3Add(&base->position, &gSpawnOffset[base->minionCount], &minionTransform.position);
                    levelSceneSpawnMinion(&gCurrentLevel, MinionTypeMelee, &minionTransform, base->baseId, base->team.teamNumber);
                    ++base->minionCount;

                    base->stateTimeLeft = SPAWN_TIME;
                }
            }
            break;
        case LevelBaseStateUpgradingSpawnRate:
        case LevelBaseStateUpgradingCapacity:
        case LevelBaseStateUpgradingDefence:
            base->stateTimeLeft -= gTimeDelta;

            if (base->stateTimeLeft <= 0.0f) {
                switch (base->state) {
                    case LevelBaseStateUpgradingSpawnRate:
                        ++base->speedUpgrade;
                        break;
                    case LevelBaseStateUpgradingCapacity:
                        ++base->capacityUpgrade;
                        break;
                    case LevelBaseStateUpgradingDefence:
                        ++base->defenseUpgrade;
                        break;
                }

                base->stateTimeLeft = SPAWN_TIME;
                base->state = LevelBaseStateSpawning;
            }
            break;
    };
}

void levelBaseRender(struct LevelBase* base, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 3);

    if (!matrix) {
        return;
    }

    struct Transform poleTransform;
    poleTransform.position = base->position;
    quatIdent(&poleTransform.rotation);
    poleTransform.scale = gOneVec;
    
    poleTransform.scale.x = gCapacityScalar[base->capacityUpgrade];
    poleTransform.scale.z = gCapacityScalar[base->capacityUpgrade];

    transformToMatrixL(&poleTransform, &matrix[0]);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(&matrix[0]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    struct Coloru8 color = gTeamColors[base->team.teamNumber];

    if (base->state == LevelBaseStateSpawning) {
        colorU8Lerp(&gTeamDarkColors[base->team.teamNumber], &color, cosf(base->stateTimeLeft * (FLASHES_PER_SPAWN * 2.0f * M_PI / SPAWN_TIME)) * 0.5f + 0.5f, &color);
    } else if (base->state == LevelBaseStateNeutral) {
        color = gTeamColors[TEAM_NONE];
    } else {
        color = gTeamDarkColors[base->team.teamNumber];
    }

    gDPSetPrimColor(renderState->dl++, 255, 255, color.r, color.g, color.b, color.a);
    gSPDisplayList(renderState->dl++, base_BasePad_mesh);
    gSPPopMatrix(renderState->dl++, 1);

    vector3Add(&poleTransform.position, &gFlagOffset, &poleTransform.position);
    poleTransform.scale = gOneVec;
    poleTransform.scale.y = 1.0f / gSpawnTimeCaptureScalar[base->defenseUpgrade];
    transformToMatrixL(&poleTransform, &matrix[1]);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(&matrix[1]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, base_flag_pole_Pole_mesh);
    gSPPopMatrix(renderState->dl++, 1);

    if (base->team.teamNumber != TEAM_NONE) {
        poleTransform.position.y += SCENE_SCALE * poleTransform.scale.y * mathfLerp(MIN_FLAG_HEIGHT, MAX_FLAG_HEIGHT, base->captureProgress / CAPTURE_TIME);
        poleTransform.scale.y = 1.0f;
        quatAxisAngle(&gUp, cosf(gTimePassed * 3.0f) * 0.25f + cosf(gTimePassed * 5.0f) * 0.125f + cosf(gTimePassed * 7.0f) * 0.125f, &poleTransform.rotation);
        transformToMatrixL(&poleTransform, &matrix[2]);
        gSPMatrix(renderState->dl++, osVirtualToPhysical(&matrix[2]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        color = gTeamColors[base->team.teamNumber];
        gDPSetPrimColor(renderState->dl++, 255, 255, color.r, color.g, color.b, color.a);
        gSPDisplayList(renderState->dl++, base_flag_Flag_mesh);
        gSPPopMatrix(renderState->dl++, 1);
    }
}

void levelBaseReleaseMinion(struct LevelBase* base) {
    --base->minionCount;
}