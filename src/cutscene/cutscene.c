#include "cutscene.h"
#include "util/memory.h"
#include "graphics/gfx.h"
#include "../data/models/characters.h"
#include "scene/team_data.h"
#include "sk64/skelatool_defs.h"
#include "scene/faction.h"
#include "scene/scene_management.h"
#include "controls/controller.h"

#include "../data/cutscenes/geometry_set.h"
#include "../data/level_themes/Mars/theme.h"
#include "../data/cutscenes/geometry_animdef.inc.h"

struct Cutscene gCutscene;

struct Transform gCameraPos = {
    {-66.6263f * SCENE_SCALE, -6.69764f * SCENE_SCALE, 1.71001f * SCENE_SCALE},
    {0.577695f, 0.276208f, 0.314765f, 0.700643f},
    // {0.0f, 0.0f, 100 * SCENE_SCALE},
    {1.0f, 1.0f, 1.0f},
};

struct Quaternion gCameraRotation = {0.707107f, 0.0f, 0.0f, 0.707107f};

struct Transform gRelativeCamera = {
    {638.493652f, -897.510986f, 564.948242f},
    {-0.051174067, 0.238761738, 0.0129388198, 0.96964246},
    {1.0f, 1.0f, 1.0f},
};

Lights1 gCutsceneLights = gdSPDefLights1(
	0x55, 0x61, 0x67,
	0xAB, 0xC3, 0xCF, 0x28, 0x28, 0x28);

char gDogMinionBones[] = {
    CUTSCENE_ANIMATIONS_MINION_001_BONE,
    CUTSCENE_ANIMATIONS_MINION_002_BONE,
    CUTSCENE_ANIMATIONS_MINION_003_BONE,
};

char gCatMinionBones[] = {
    CUTSCENE_ANIMATIONS_MINION_004_BONE,
    CUTSCENE_ANIMATIONS_MINION_005_BONE,
    CUTSCENE_ANIMATIONS_MINION_006_BONE,
};

void cutsceneAnimationEvent(struct SKAnimator* animator, void* data, struct SKAnimationEvent* event) {
    if (event->id == SK_ANIMATION_EVENT_END) {
        sceneEndCutscene();
    }
}

void cutsceneInit(struct Cutscene* cutscene, unsigned animationIndex) {
    skAnimatorInit(&cutscene->animator, CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT, cutsceneAnimationEvent, cutscene);

    cutscene->rootTransforms = malloc(sizeof(struct Transform) * CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT);
    for (unsigned i = 0; i < CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT; ++i) {
        transformInitIdentity(&cutscene->rootTransforms[i]);
    }

    cameraInit(&cutscene->camera, 45.0f, 100.0f, 18000.0f);

    skAnimatorRunClip(&cutscene->animator, &cutscene_animations_animations[animationIndex], 0);
}

void cutsceneUpdate(struct Cutscene* cutscene) {
    skAnimatorUpdate(&cutscene->animator, cutscene->rootTransforms, 1.0f);

    if (controllerGetButtonDown(0, START_BUTTON)) {
        sceneEndCutscene();
    }
}

void cutsceneRender(struct Cutscene* cutscene, struct RenderState* renderState) {
    transformConcat(&cutscene->rootTransforms[CUTSCENE_ANIMATIONS_CAMERA_BONE], &gRelativeCamera, &cutscene->camera.transform);

    gDPPipeSync(renderState->dl++);
    Vp* viewport = &gFullScreenVP;
    cameraSetupMatrices(
        &cutscene->camera, 
        renderState, 
        (float)viewport->vp.vscale[0] / (float)viewport->vp.vscale[1],
        0.0f
    );
    gSPViewport(renderState->dl++, osVirtualToPhysical(&gFullScreenVP));
    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );

    Mtx* skyboxMatrix = renderStateRequestMatrices(renderState, 1);
    struct Transform skyboxTransform;
    skyboxTransform.position = cutscene->camera.transform.position;
    quatIdent(&skyboxTransform.rotation);
    skyboxTransform.scale = gOneVec;
    
    transformToMatrixL(&skyboxTransform, skyboxMatrix);
    gDPPipeSync(renderState->dl++);
    gSPClearGeometryMode(renderState->dl++, G_ZBUFFER | G_LIGHTING | G_CULL_BOTH);
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gSPMatrix(renderState->dl++, skyboxMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, Mars_SSSkybox);
    gSPDisplayList(renderState->dl++, Mars_SkyboxDisplayList);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    gDPPipeSync(renderState->dl++);

    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER | G_CULL_BACK);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPSetLights1(renderState->dl++, gCutsceneLights);

    gSPDisplayList(renderState->dl++, cutscene_set_model_gfx);

    Mtx* boneMatrices = renderStateRequestMatrices(renderState, CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT);
    Mtx* minonScale = renderStateRequestMatrices(renderState, 1);
    guScale(minonScale, MINION_RENDER_SCALE, MINION_RENDER_SCALE, MINION_RENDER_SCALE);

    for (unsigned i = 0; i < CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT; ++i) {
        transformToMatrixL(&cutscene->rootTransforms[i], &boneMatrices[i]);
    }

    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, gTeamTexture);
    gDPUseTeamPallete(renderState->dl++, 0, 2);
    gSPSegment(
        renderState->dl++, 
        MATRIX_TRANSFORM_SEGMENT,  
        osVirtualToPhysical(&boneMatrices[CUTSCENE_ANIMATIONS_ROOT_BONE])
    );
    gSPSetGeometryMode(renderState->dl++, G_LIGHTING);
    // + 4 to skip culling
    gSPDisplayList(renderState->dl++, doglow_DogLow_mesh + 4);

    for (unsigned i = 0; i < sizeof(gDogMinionBones)/sizeof(gDogMinionBones[0]); ++i) {
        gDPPipeSync(renderState->dl++);
        gSPMatrix(renderState->dl++, &boneMatrices[(unsigned)gDogMinionBones[i]], G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPMatrix(renderState->dl++, minonScale, G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_MUL);
        gDPUseTeamPallete(renderState->dl++, 0, 0);
        gSPDisplayList(renderState->dl++, Minion_DogMinion_mesh);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }

    gDPPipeSync(renderState->dl++);
    gDPUseTeamPallete(renderState->dl++, 1, 2);
    gSPSegment(
        renderState->dl++, 
        MATRIX_TRANSFORM_SEGMENT,  
        osVirtualToPhysical(&boneMatrices[CUTSCENE_ANIMATIONS_ROOTC_BONE])
    );
    // + 4 to skip culling
    gSPDisplayList(renderState->dl++, catlow_CatLow_mesh + 4
    );
    gDPPipeSync(renderState->dl++);

    for (unsigned i = 0; i < sizeof(gCatMinionBones)/sizeof(gCatMinionBones[0]); ++i) {
        gDPPipeSync(renderState->dl++);
        gSPMatrix(renderState->dl++, &boneMatrices[(unsigned)gCatMinionBones[i]], G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPMatrix(renderState->dl++, minonScale, G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_MUL);
        gDPUseTeamPallete(renderState->dl++, 1, 0);
        gSPDisplayList(renderState->dl++, Minion_CatMinion_mesh);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }
}