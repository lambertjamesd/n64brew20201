
#include "minimap.h"
#include "graphics/sprite.h"
#include "level_scene.h"
#include "team_data.h"

struct MinimapTransform {
    float scaleX;
    float scaleY;
    float offsetX;
    float offsetY;
};

void minimapApplyTransform(struct MinimapTransform* transform, struct Vector3* input, struct Vector2* output) {
    output->x = input->x * transform->scaleX + transform->offsetX;
    output->y = input->z * transform->scaleY + transform->offsetY;
}

void minimapRenderDot(struct RenderState* renderState, struct MinimapTransform* transform, struct Vector3* at, int width) {
    struct Vector2 screenPos;
    minimapApplyTransform(transform, at, &screenPos);
    spriteDraw(
        renderState, LAYER_SOLID_COLOR, 
        (int)screenPos.x - (width >> 1), (int)screenPos.y - (width >> 1), width, width,
        0, 0, 0, 0
    );
}

void minimapRender(struct LevelScene* scene, struct RenderState* renderState, unsigned short* minimapLocation) {
    spriteSetColor(renderState, LAYER_SOLID_COLOR, 0, 0, 0, 128);
    spriteDraw(
        renderState, LAYER_SOLID_COLOR, 
        minimapLocation[0], minimapLocation[1], minimapLocation[2] - minimapLocation[0], minimapLocation[3] - minimapLocation[1],
        0, 0, 0, 0
    );

    struct MinimapTransform transform;

    transform.scaleX = (minimapLocation[2] - minimapLocation[0]) / (scene->definition->levelBoundaries.max.x - scene->definition->levelBoundaries.min.x);
    transform.scaleY = (minimapLocation[3] - minimapLocation[1]) / (scene->definition->levelBoundaries.max.y - scene->definition->levelBoundaries.min.y);

    transform.offsetX = minimapLocation[0] - scene->definition->levelBoundaries.min.x * transform.scaleX;
    transform.offsetY = minimapLocation[1] - scene->definition->levelBoundaries.min.y * transform.scaleY;

    for (unsigned i = 0; i < scene->baseCount; ++i) {
        // TODO flashing
        struct Coloru8 color = gTeamColors[levelBaseGetFactionID(&scene->bases[i])];
        spriteSetColor(renderState, LAYER_SOLID_COLOR, color.r, color.g, color.b, color.a);
        minimapRenderDot(renderState, &transform, &scene->bases[i].position, 2);
    }

    for (unsigned i = 0; i < scene->minionCount; ++i) {
        if (scene->minions[i].minionFlags & MinionFlagsActive) {
            struct Coloru8 color = gTeamColors[scene->minions[i].team.teamNumber];
            spriteSetColor(renderState, LAYER_SOLID_COLOR, color.r, color.g, color.b, color.a);
            minimapRenderDot(renderState, &transform, &scene->minions[i].transform.position, 1);
        }
    }

    for (unsigned i = 0; i < scene->playerCount; ++i) {
        struct Coloru8 color = gTeamColors[scene->players[i].team.teamNumber];
        spriteSetColor(renderState, LAYER_SOLID_COLOR, color.r, color.g, color.b, color.a);
        minimapRenderDot(renderState, &transform, &scene->players[i].transform.position, 3);
    }
}