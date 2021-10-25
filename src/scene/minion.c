
#include <ultra64.h>

#include "minion.h"
#include "util/rom.h"
#include "graphics/gfx.h"
#include "util/time.h"
#include <stdlib.h>

#include "sk64/skelatool_clip.h"
#include "../data/models/characters.h"
#include "scene_management.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "scene_management.h"
#include "team_data.h"
#include "minion_animations.h"

#define MINION_FOLLOW_DIST  3.0f
#define MINION_MOVE_SPEED   (PLAYER_MOVE_SPEED * 0.9f)
#define MINION_ACCELERATION PLAYER_MOVE_ACCELERATION
#define MINION_HP           2
#define MINION_DPS          1

struct CollisionCircle gMinionCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * 0.4f,
};

struct MinionDef {
    Gfx* dl;
    unsigned boneCount;
    struct Transform* defaultPose;
};

void minionCorrectOverlap(struct DynamicSceneOverlap* overlap) {
    teamEntityCorrectOverlap(overlap);

    if (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) {
        struct Minion* entityA = (struct Minion*)overlap->thisEntry->data;
        struct TeamEntity* entityB = (struct TeamEntity*)overlap->otherEntry->data;

        if (entityB == entityA->attackTarget && (entityA->minionFlags & (MinionFlagsAttacking | MinionFlagsAttacked)) == MinionFlagsAttacking) {
            teamEntityApplyDamage(entityB, MINION_DPS * skAnimationLength(&minion_animations[MINION_ANIMATION_ATTACK]));
        } else if (entityB->teamNumber != entityA->team.teamNumber && entityA->attackTarget == 0) {
            minionSetAttackTarget(entityA, entityB);
        }
    }
}

void minionAnimationEvent(struct SKAnimator* animator, void* data, struct SKAnimationEvent* event) {
    struct Minion* minion = (struct Minion*)data;

    if (event->id == MINION_ANIMATION_EVENT_ATTACK) {
        minion->minionFlags |= MinionFlagsAttacking;
    }
}

void minionInit(struct Minion* minion, enum MinionType type, struct Transform* at, unsigned char sourceBaseId, unsigned team, enum MinionCommand defualtCommand) {
    minion->team.teamNumber = team;
    minion->team.entityType = TeamEntityTypeMinion;

    minion->transform = *at;
    minion->minionType = type;
    minion->minionFlags = MinionFlagsActive;
    minion->sourceBaseId = sourceBaseId;
    minion->velocity = gZeroVec;
    minion->hp = MINION_HP;

    struct Vector2 position;

    position.x = minion->transform.position.x;
    position.y = minion->transform.position.z;

    minion->collider = dynamicSceneNewEntry(
        &gMinionCollider.shapeCommon, 
        minion, 
        &position,
        minionCorrectOverlap,
        DynamicSceneEntryHasTeam,
        CollisionLayersTangible | CollisionLayersBase | COLLISION_LAYER_FOR_TEAM(team)
    );

    quatAxisAngle(&gUp, M_PI * 2.0f * rand() / RAND_MAX, &minion->transform.rotation);

    minion->currentCommand = -1;
    minionIssueCommand(minion, defualtCommand);

    skAnimatorInit(&minion->animator, 1, minionAnimationEvent, minion);
    skAnimatorRunClip(&minion->animator, &minion_animations[MINION_ANIMATION_IDLE], SKAnimatorFlagsLoop);
    transformInitIdentity(&minion->animationTransform);
}

void minionRender(struct Minion* minion, struct RenderState* renderState) {
    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    struct Transform finalTransform;
    transformConcat(&minion->transform, &minion->animationTransform, &finalTransform);
    transformToMatrixL(&finalTransform, matrix);
    gDPSetPrimColor(renderState->dl++, 255, 255, gTeamColors[minion->team.teamNumber].r, gTeamColors[minion->team.teamNumber].g, gTeamColors[minion->team.teamNumber].b, gTeamColors[minion->team.teamNumber].a);
    gSPMatrix(renderState->dl++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, DogMinion_Dog_001_mesh);
    gSPPopMatrix(renderState->dl++, 1);
}

void minionIssueCommand(struct Minion* minion, enum MinionCommand command) {
    if (command == MinionCommandDefend) {
        minion->defensePoint = minion->transform.position;
    }
    
    minion->currentTarget = 0;
    minion->currentCommand = command;
    minion->attackTarget = 0;
}
    
void minionUpdate(struct Minion* minion) {
    struct Vector3* target;
    float minDistance = 0.0f;

    if (minion->hp <= 0) {
        minionCleanup(minion);
        return;
    }

    switch (minion->currentCommand) {
        case MinionCommandFollow:
            target = &gCurrentLevel.players[minion->team.teamNumber].transform.position;
            minDistance = MINION_FOLLOW_DIST * SCENE_SCALE;
            break;
        case MinionCommandDefend:
            target = teamEntityGetPosition(minion->currentTarget);
        
            if (target != 0 && vector3DistSqrd(target, &minion->defensePoint) > MINION_DEFENSE_RADIUS * MINION_DEFENSE_RADIUS) {
                minion->currentTarget = 0;
                target = 0;
            }

            if (target == 0) {
                target = &minion->defensePoint;
            }

            break;
        case MinionCommandAttack:
            target = teamEntityGetPosition(minion->currentTarget);

            if (minion->currentTarget && minion->currentTarget->entityType == TeamEntityTypeBase) {
                minDistance = 1.0f;
            }

            break;
    }

    if (minion->attackTarget) {
        if ((minion->minionFlags & MinionFlagsAttacking) != 0) {
            minion->minionFlags |= MinionFlagsAttacked;
        }

        if (!skAnimatorIsRunning(&minion->animator)) {
            minion->minionFlags &= ~(MinionFlagsAttacking | MinionFlagsAttacked);
            minion->attackTarget = 0;
            skAnimatorRunClip(&minion->animator, &minion_animations[MINION_ANIMATION_IDLE], SKAnimatorFlagsLoop);
        } else {
            target = teamEntityGetPosition(minion->attackTarget);
            minDistance = 0.0f;
        }
    }

    struct Vector3 targetVelocity = gZeroVec;

    if (target) {
        struct Vector3 offset;
        vector3Sub(target, &minion->transform.position, &offset);

        float distSqr = vector3MagSqrd(&offset);

        if (distSqr > minDistance * minDistance) {
            vector3Scale(&offset, &offset, 1.0f / sqrtf(distSqr));
            vector3Scale(&offset, &targetVelocity, MINION_MOVE_SPEED);
        }
    }

    vector3MoveTowards(&minion->velocity, &targetVelocity, MINION_ACCELERATION * gTimeDelta, &minion->velocity);
    vector3Scale(&minion->velocity, &minion->velocity, 0.9f);
    vector3AddScaled(&minion->transform.position, &minion->velocity, SCENE_SCALE * gTimeDelta, &minion->transform.position);

    struct Vector2 right;
    right.x = minion->velocity.z;
    right.y = minion->velocity.x;

    if (right.x != 0.0f || right.y != 0.0f) {
        vector2Normalize(&right, &right);
        quatAxisComplex(&gUp, &right, &minion->transform.rotation);
    }

    minion->collider->center.x = minion->transform.position.x;
    minion->collider->center.y = minion->transform.position.z;

    skAnimatorUpdate(&minion->animator, &minion->animationTransform, 1.0f);
}

void minionCleanup(struct Minion* minion) {
    if (minion->minionFlags & MinionFlagsActive) {
        minion->minionFlags = 0;
        levelBaseReleaseMinion(&gCurrentLevel.bases[minion->sourceBaseId]);
        dynamicSceneDeleteEntry(minion->collider);
    }
}

void minionSetAttackTarget(struct Minion* minion, struct TeamEntity* target) {
    minion->attackTarget = target;
    minion->minionFlags &= ~(MinionFlagsAttacking | MinionFlagsAttacked);
    skAnimatorRunClip(&minion->animator, &minion_animations[MINION_ANIMATION_ATTACK], 0);
}