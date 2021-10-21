#include "recall_circle.h"
#include "collision/circle.h"
#include "game_defs.h"
#include "collision/collisionlayers.h"
#include "teamentity.h"
#include "minion.h"
#include "team_data.h"
#include "../data/models/characters.h"

#define RECALL_RADIUS   3.0f

struct CollisionCircle gRecallCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE * RECALL_RADIUS,
};

void recallCircleInit(struct RecallCircle* circle) {
    circle->collider = 0;
}

void recallCircleOnCollider(struct DynamicSceneOverlap* overlap) {
    if (overlap->otherEntry->flags & DynamicSceneEntryHasTeam) {
        struct TeamEntity* teamEntity = (struct TeamEntity*)overlap->otherEntry->data;

        if (teamEntity->entityType == TeamEntityTypeMinion) {
            minionIssueCommand((struct Minion*)teamEntity, MinionCommandFollow);
        }
    }
}

void recallCircleActivate(struct RecallCircle* circle, struct Vector2* at, unsigned team) {
    if (!circle->collider) {
        circle->collider = dynamicSceneNewEntry(
            &gRecallCollider.shapeCommon,
            circle,
            at,
            recallCircleOnCollider,
            DynamicSceneEntryIsTrigger,
            COLLISION_LAYER_FOR_TEAM(team)
        );
    } else {
        circle->collider->center = *at;
    }
}

void recallCircleDisable(struct RecallCircle* circle) {
    if (circle->collider) {
        dynamicSceneDeleteEntry(circle->collider);
        circle->collider = 0;
    }
}

void recallCircleRender(struct RecallCircle* circle, struct RenderState* renderState, unsigned team) {
    if (!circle->collider) {
        return;
    }

    Mtx* matrix = renderStateRequestMatrices(renderState, 1);

    if (!matrix) {
        return;
    }

    struct Transform transform;
    transform.position.x = circle->collider->center.x;
    transform.position.y = FLOOR_HEIGHT;
    transform.position.z = circle->collider->center.y;
    quatIdent(&transform.rotation);
    vector3Scale(&gOneVec, &transform.scale, RECALL_RADIUS);
    transformToMatrixL(&transform, matrix);

    gSPMatrix(renderState->transparentDL++, osVirtualToPhysical(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
    struct Coloru8 color = gTeamColors[team];
    gDPSetPrimColor(renderState->transparentDL++, 255, 255, color.r, color.g, color.b, color.a);
    gSPDisplayList(renderState->transparentDL++, RecallCircle_Cylinder_mesh);
    gSPPopMatrix(renderState->transparentDL++, 1);
}