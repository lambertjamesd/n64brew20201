
#include "minimap.h"
#include "graphics/sprite.h"
#include "level_scene.h"
#include "team_data.h"
#include "util/time.h"
#include "math/mathf.h"

Vtx gPointerVertex[] = {
    {{{-400, 0, 600}, 0, {0, 0}, {0, 0, 0, 0}}},
    {{{0, 0, -600}, 0, {0, 0}, {0, 0, 0, 0}}},
    {{{400, 0, 600}, 0, {0, 0}, {0, 0, 0, 0}}},
};

struct MinimapTransform {
    float scaleX;
    float scaleY;
    float offsetX;
    float offsetY;
};

#define CAPTURE_FLASH_FREQ  0.5f

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
    spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
    spriteSolid(
        renderState, LAYER_SOLID_COLOR, 
        minimapLocation[0], minimapLocation[1], minimapLocation[2] - minimapLocation[0], minimapLocation[3] - minimapLocation[1]
        // 0, 0, 0, 0
    );

    struct MinimapTransform transform;

    transform.scaleX = (minimapLocation[2] - minimapLocation[0]) / (scene->definition->levelBoundaries.max.x - scene->definition->levelBoundaries.min.x);
    transform.scaleY = (minimapLocation[3] - minimapLocation[1]) / (scene->definition->levelBoundaries.max.y - scene->definition->levelBoundaries.min.y);

    transform.offsetX = minimapLocation[0] - scene->definition->levelBoundaries.min.x * transform.scaleX;
    transform.offsetY = minimapLocation[1] - scene->definition->levelBoundaries.min.y * transform.scaleY;

    for (unsigned i = 0; i < scene->baseCount; ++i) {
        struct Coloru8 color;
        struct LevelBase* base = &scene->bases[i];

        if (levelBaseIsBeingCaptured(base) && mathfMod(gTimePassed, CAPTURE_FLASH_FREQ) > CAPTURE_FLASH_FREQ * 0.5f) {
            color = gColorBlack;
        } else if (base->state == LevelBaseStateNeutral || base->state == LevelBaseStateSpawning)  {
            color = gTeamColors[levelBaseGetTeam(base)];
        } else {
            color = gTeamDarkColors[levelBaseGetTeam(base)];
        }

        spriteSetColor(renderState, LAYER_SOLID_COLOR, color);
        minimapRenderDot(renderState, &transform, &base->position, 4);
    }

    for (unsigned i = 0; i < scene->minionCount; ++i) {
        if (scene->minions[i].minionFlags & MinionFlagsActive) {
            struct Coloru8 color = gTeamColors[scene->minions[i].team.teamNumber];
            spriteSetColor(renderState, LAYER_SOLID_COLOR, color);
            minimapRenderDot(renderState, &transform, &scene->minions[i].transform.position, 2);
        }
    }

    Mtx* minimapTransform = renderStateRequestMatrices(renderState, 1);

    if (!minimapTransform) {
        return;
    }

    float minimapMtx[4][4];
    guScaleF(minimapMtx, transform.scaleX, 1.0f, transform.scaleY);
    minimapMtx[3][0] = transform.offsetX;
    minimapMtx[3][2] = transform.offsetY;


    guMtxF2L(minimapMtx, minimapTransform);

    Gfx tmpBuffer[8];
    Gfx* curr = tmpBuffer;
    gSPMatrix(curr++, minimapTransform, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPClearGeometryMode(curr++, G_CULL_BOTH);
    spriteWriteRaw(renderState, LAYER_SOLID_COLOR, tmpBuffer, curr - tmpBuffer);

    for (unsigned i = 0; i < scene->playerCount; ++i) {
        struct Coloru8 color = gTeamColors[scene->players[i].team.teamNumber];
        spriteSetColor(renderState, LAYER_SOLID_COLOR, color);

        Mtx* matrix = renderStateRequestMatrices(renderState, 1);

        if (!matrix) {
            continue;
        }

        transformToMatrixL(&scene->players[i].transform, matrix);
        curr = tmpBuffer;
        gSPMatrix(curr++, matrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPVertex(curr++, gPointerVertex, 3, 0);
        gSP1Triangle(curr++, 0, 1, 2, 0);
        gSPPopMatrix(curr++, G_MTX_MODELVIEW);
        spriteWriteRaw(renderState, LAYER_SOLID_COLOR, tmpBuffer, curr - tmpBuffer);
    }

    curr = tmpBuffer;
    gSPPopMatrix(curr++, G_MTX_MODELVIEW);
    spriteWriteRaw(renderState, LAYER_SOLID_COLOR, tmpBuffer, curr - tmpBuffer);
}