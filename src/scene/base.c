
#include "base.h"
#include "collision/circle.h"
#include "collision/collisionlayers.h"
#include "../data/models/characters.h"

#include "game_defs.h"

struct CollisionCircle gBaseCollider = {
    {CollisionShapeTypeCircle},
    SCENE_SCALE,
};

void levelBaseTrigger(struct DynamicSceneOverlap* overlap) {
    
}

void levelBaseInit(struct LevelBase* base, struct Vector2* position) {
    base->position = *position;

    base->collider = dynamicSceneNewEntry(&gBaseCollider.shapeCommon, base);

    base->collider->center.x = position->x;
    base->collider->center.y = position->y;
    base->collider->onCollide = levelBaseTrigger;
    base->collider->flags = DynamicSceneEntryIsTrigger;
    base->collider->collisionLayers = CollisionLayersBase;
}

void levelBaseUpdate(struct LevelBase* base) {

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