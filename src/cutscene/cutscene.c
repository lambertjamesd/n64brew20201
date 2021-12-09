#include "cutscene.h"
#include "util/memory.h"
#include "graphics/gfx.h"
#include "../data/models/characters.h"
#include "scene/team_data.h"
#include "sk64/skelatool_defs.h"
#include "scene/faction.h"
#include "scene/scene_management.h"
#include "controls/controller.h"
#include "audio/soundplayer.h"
#include "util/time.h"
#include "menu/textbox.h"
#include "graphics/sprite.h"
#include "menu/kickflipfont.h"

#include "../data/cutscenes/geometry_ending.h"

#include "../data/cutscenes/geometry_ss_set.h"
#include "../data/cutscenes/geometry_surface_set.h"
#include "../data/level_themes/Space/theme.h"
#include "../data/cutscenes/geometry_animdef.inc.h"
#include "../data/cutscenes/geometry_ending_animdef.inc.h"

#include "../data/fonts/fonts.h"
#include "../data/gameplaymenu/menu.h"

#define SET_SET_EVENT               0x1000
#define PLAY_SOUND_EVENT            0x2000
#define STOP_SOUND_EVENT            0x3000
#define PLAY_TRANSITION_SOUND_EVENT 0x4000
#define FADE_TO_BLACK               0x5000

#define FADE_IN_TIME            0.5f
#define FADE_OUT_TIME           2.0f

#define CREATE_SCENE_EVENT(event, sceneId)     ((event) | ((0xfff) & (sceneId)))
#define GET_SCENE_EVENT_TYPE(event)             (0xf000 & (event))
#define GET_SCENE_EVENT_DATA(event)             (0xfff & (event))

#define SKIP_BUTTON_TIME            1.5f

extern Gfx cutscene_console_model_gfx[];

struct Cutscene gCutscene;

struct SKAnimationHeader* gCutsceneAnimations[] = {
    [CutsceneIndexIntro] = &cutscene_animations_animations[CUTSCENE_ANIMATIONS_CUTSCENE_ANIMATIONS_CUTCENE_001_INTROCUTSCENE_INDEX],
    [CutsceneIndexEnding] = &cutscene_ending_animations[CUTSCENE_ENDING_CUTSCENE_ENDING_CUTCENE_001_ENDINGCUTSCENE_INDEX],
    [CutsceneIndexEndingCat] = &cutscene_ending_animations[CUTSCENE_ENDING_CUTSCENE_ENDING_CUTCENE_001_ENDINGCUTSCENECAT_INDEX],
};

struct Transform gRelativeCamera[] = {
    [CutsceneIndexIntro] = {
        {622.375, -897.390747, 541.383057},
        {-0.0511746705, 0.238762602, 0.0129380375, 0.969641984},
        {1.0f, 1.0f, 1.0f},
    },
    [CutsceneIndexEnding] = {
        {-10.3515625, -2.70001221, 18.1567993},
        {0.107586406, -0.243552148, -0.117358007, 0.956730902},
        {1.0f, 1.0f, 1.0f},
    },
    [CutsceneIndexEndingCat] = {
        {-10.3515625, -2.70001221, 18.1567993},
        {0.107586406, -0.243552148, -0.117358007, 0.956730902},
        {1.0f, 1.0f, 1.0f},
    },
};

char gPalleteSwap[] = {
    0, 0, 1,
};

struct Transform gCameraPos = {
    {-6.07838f * SCENE_SCALE, -2.41124f * SCENE_SCALE, 6.18475f * SCENE_SCALE},
    {0.607257, -0.326321, -0.357687, 0.629931},
    {1.0f, 1.0f, 1.0f},
};

struct Quaternion gSkyboxRotation = {
    0.707106781f, 0.0f, 0.0f, 0.707106781f,
};

struct SKAnimationEvent gIntroEvents[] = {
    {0, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_LEVELMUSIC_DANAIDES)},
    {0, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_ITEMWAIT)},
    {104, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_DOG_BARK_2)},
    {110, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_DOG_BARK_7)},
    {115, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_DOG_BARK_3)},
    {121, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_DOG_BARK_4)},
    {122, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_DOG_PANT_IDLE_1)},
    {192, CREATE_SCENE_EVENT(STOP_SOUND_EVENT, SOUNDS_ITEMWAIT)},
    {192, CREATE_SCENE_EVENT(SET_SET_EVENT, 0x3)},
    {195, CREATE_SCENE_EVENT(SET_SET_EVENT, 0x2)},
    {237, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_SPAWN)},
    {267, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_UI_SELECT3)},
    {274, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_UI_SELECT2)},
    {280, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_UI_SELECT3)},
    {413, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_DOG_JUMP_GRUNT_5)},
    {413, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_CATMEOW1)},
    {425, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_FOOTSTEP0)},
    {430, CREATE_SCENE_EVENT(PLAY_SOUND_EVENT, SOUNDS_FOOTSTEP0)},
};

struct SKAnimationEvent gEndingEvents[] = {
    {0, CREATE_SCENE_EVENT(PLAY_TRANSITION_SOUND_EVENT, SOUNDS_LEVELMUSIC_FERMIPARADOX_MASTERED)},
    {506, CREATE_SCENE_EVENT(FADE_TO_BLACK, 0)},
};

struct SKAnimationEvent gEndingCatEvents[] = {
    {0, CREATE_SCENE_EVENT(PLAY_TRANSITION_SOUND_EVENT, SOUNDS_LEVELMUSIC_FERMIPARADOX_MASTERED)},
    {506, CREATE_SCENE_EVENT(FADE_TO_BLACK, 0)},
};

Gfx* gCutsceneSets[] = {
    cutscene_ss_set_model_gfx,
    cutscene_surface_set_model_gfx,
};

unsigned short gStartingSceneMask[] = {
    [CutsceneIndexIntro] = 0x1,
    [CutsceneIndexEnding] = 0x2,
    [CutsceneIndexEndingCat] = 0x2,
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

int skipIconRenderer(struct RenderState* renderState, void* data, int x, int y) {
    if (renderState) {
        spriteDraw(renderState, LAYER_BUTTONS, x, y, 16, 16, 0, 48, 0, 0);
    }

    return 24;
}

void cutsceneAnimationEvent(struct SKAnimator* animator, void* data, struct SKAnimationEvent* event) {
    struct Cutscene* cutscene = (struct Cutscene*)animator->eventCallbackData;
    
    if (event->id == SK_ANIMATION_EVENT_END) {
        sceneEndCutscene();
    }

    unsigned eventType = GET_SCENE_EVENT_TYPE(event->id);

    switch (eventType) {
        case SET_SET_EVENT:
            cutscene->currentSetMask = GET_SCENE_EVENT_DATA(event->id);
            break;
        case PLAY_SOUND_EVENT:
            soundPlayerPlay(GET_SCENE_EVENT_DATA(event->id), 1.0f, 1.0f, SoundPlayerPriorityBackground, 0, 0);
            break;
        case PLAY_TRANSITION_SOUND_EVENT:
            soundPlayerPlay(GET_SCENE_EVENT_DATA(event->id), 1.0f, 1.0f, SoundPlayerPriorityMusic, SoundPlayerFlagsTransition, 0);
            break;
        case STOP_SOUND_EVENT:
            soundPlayerStopWithClipId(GET_SCENE_EVENT_DATA(event->id));
            break;
        case FADE_TO_BLACK:
            cutscene->targetFade = 0.0f;
            break;
    }
}

void cutsceneInit(struct Cutscene* cutscene, enum CutsceneIndex index) {
    cutscene->cutsceneIndex = index;
    gCutsceneAnimations[CutsceneIndexIntro]->animationEvents = gIntroEvents;
    gCutsceneAnimations[CutsceneIndexIntro]->numEvents = sizeof(gIntroEvents) / sizeof(*gIntroEvents);

    gCutsceneAnimations[CutsceneIndexEnding]->animationEvents = gEndingEvents;
    gCutsceneAnimations[CutsceneIndexEnding]->numEvents = sizeof(gEndingEvents) / sizeof(*gEndingEvents);

    gCutsceneAnimations[CutsceneIndexEndingCat]->animationEvents = gEndingCatEvents;
    gCutsceneAnimations[CutsceneIndexEndingCat]->numEvents = sizeof(gEndingCatEvents) / sizeof(*gEndingCatEvents);

    skAnimatorInit(&cutscene->animator, CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT, cutsceneAnimationEvent, cutscene);

    cutscene->rootTransforms = malloc(sizeof(struct Transform) * CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT);
    for (unsigned i = 0; i < CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT; ++i) {
        transformInitIdentity(&cutscene->rootTransforms[i]);
    }

    cameraInit(&cutscene->camera, 45.0f, 5.0f, 8000.0f);
    initKickflipFont();

    skAnimatorRunClip(&cutscene->animator, gCutsceneAnimations[index], 0);
    
    cutscene->currentSetMask = gStartingSceneMask[index];
    cutscene->currentFade = 0.0f;
    cutscene->targetFade = 1.0f;
}

void cutsceneUpdate(struct Cutscene* cutscene) {
    skAnimatorUpdate(&cutscene->animator, cutscene->rootTransforms, 1.0f);

    if (controllerGetButtonDown(0, START_BUTTON)) {
        if (textBoxVisiblePercent(&gTextBox) == 1.0f) {
            sceneEndCutscene();
        } else if (gTextBox.nextState == TextBoxStateHidden) {
            cutscene->skipTimer = SKIP_BUTTON_TIME;
            textBoxInit(&gTextBox, "Skip", 100, SCREEN_WD / 2, SCREEN_HT * 2 / 3);
            textBoxSetIcon(&gTextBox, skipIconRenderer, 0);
        }
    }

    if (cutscene->skipTimer > 0) {
        cutscene->skipTimer -= gTimeDelta;

        if (cutscene->skipTimer <= 0.0f) {
            cutscene->skipTimer = 0.0f;
            textBoxHide(&gTextBox);
        }
    }

    textBoxUpdate(&gTextBox);

    if (cutscene->currentFade < cutscene->targetFade) {
        cutscene->currentFade += gTimeDelta * (1.0f / FADE_IN_TIME);

        if (cutscene->currentFade > cutscene->targetFade) {
            cutscene->currentFade = cutscene->targetFade;
        }
    }

    if (cutscene->currentFade > cutscene->targetFade) {
        cutscene->currentFade -= gTimeDelta * (1.0f / FADE_OUT_TIME);

        if (cutscene->currentFade < cutscene->targetFade) {
            cutscene->currentFade = cutscene->targetFade;
        }
    }
}

void cutsceneRender(struct Cutscene* cutscene, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gUseSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);
    spriteSetLayer(renderState, LAYER_BUTTONS, gUseButtonsIcon);

    transformConcat(&cutscene->rootTransforms[CUTSCENE_ANIMATIONS_CAMERA_BONE], &gRelativeCamera[cutscene->cutsceneIndex], &cutscene->camera.transform);

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
    skyboxTransform.rotation = gSkyboxRotation;
    skyboxTransform.scale = gOneVec;
    
    transformToMatrixL(&skyboxTransform, skyboxMatrix);
    gDPPipeSync(renderState->dl++);
    gSPClearGeometryMode(renderState->dl++, G_ZBUFFER | G_LIGHTING | G_CULL_BOTH);
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gSPMatrix(renderState->dl++, skyboxMatrix, G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, Space_SSSkybox);
    gSPDisplayList(renderState->dl++, Space_SkyboxDisplayList);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    gDPPipeSync(renderState->dl++);

    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER | G_CULL_BACK);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPSetLights1(renderState->dl++, gCutsceneLights);

    for (unsigned i = 0; i < sizeof(gCutsceneSets) / sizeof(*gCutsceneSets); ++i) {
        if (cutscene->currentSetMask & (1 << i)) {
            gSPDisplayList(renderState->dl++, gCutsceneSets[i]);
        }
    }

    Mtx* boneMatrices = renderStateRequestMatrices(renderState, CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT);
    Mtx* minonScale = renderStateRequestMatrices(renderState, 1);
    guScale(minonScale, MINION_RENDER_SCALE, MINION_RENDER_SCALE, MINION_RENDER_SCALE);

    for (unsigned i = 0; i < CUTSCENE_ANIMATIONS_DEFAULT_BONES_COUNT; ++i) {
        transformToMatrixL(&cutscene->rootTransforms[i], &boneMatrices[i]);
    }

    gDPPipeSync(renderState->dl++);
    gSPDisplayList(renderState->dl++, gTeamTexture);
    gDPUseTeamPallete(renderState->dl++, gPalleteSwap[cutscene->cutsceneIndex], 2);
    gSPSegment(
        renderState->dl++, 
        MATRIX_TRANSFORM_SEGMENT,  
        osVirtualToPhysical(&boneMatrices[CUTSCENE_ANIMATIONS_ROOT_BONE])
    );
    gSPSetGeometryMode(renderState->dl++, G_LIGHTING);
    // + 4 to skip culling
    gSPDisplayList(renderState->dl++, doglow_DogLow_mesh + 4);

    gSPDisplayList(renderState->dl++, gTeamTexture);
    for (unsigned i = 0; i < sizeof(gDogMinionBones)/sizeof(gDogMinionBones[0]); ++i) {
        gDPPipeSync(renderState->dl++);
        gSPMatrix(renderState->dl++, &boneMatrices[(unsigned)gDogMinionBones[i]], G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPMatrix(renderState->dl++, minonScale, G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_MUL);
        gDPUseTeamPallete(renderState->dl++, gPalleteSwap[cutscene->cutsceneIndex], 0);
        gSPDisplayList(renderState->dl++, Minion_DogMinion_mesh);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }

    gDPPipeSync(renderState->dl++);
    gDPUseTeamPallete(renderState->dl++, 1 ^ gPalleteSwap[cutscene->cutsceneIndex], 2);
    gSPSegment(
        renderState->dl++, 
        MATRIX_TRANSFORM_SEGMENT,  
        osVirtualToPhysical(&boneMatrices[CUTSCENE_ANIMATIONS_ROOTC_BONE])
    );
    // + 4 to skip culling
    gSPDisplayList(renderState->dl++, catlow_CatLow_mesh + 4);
    gDPPipeSync(renderState->dl++);

    gSPDisplayList(renderState->dl++, gTeamTexture);
    for (unsigned i = 0; i < sizeof(gCatMinionBones)/sizeof(gCatMinionBones[0]); ++i) {
        gDPPipeSync(renderState->dl++);
        gSPMatrix(renderState->dl++, &boneMatrices[(unsigned)gCatMinionBones[i]], G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
        gSPMatrix(renderState->dl++, minonScale, G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_MUL);
        gDPUseTeamPallete(renderState->dl++, 1 ^ gPalleteSwap[cutscene->cutsceneIndex], 0);
        gSPDisplayList(renderState->dl++, Minion_CatMinion_mesh);
        gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);
    }

    gSPMatrix(renderState->dl++, &boneMatrices[CUTSCENE_ANIMATIONS_CONSOLEFRAME_BONE], G_MTX_MODELVIEW | G_MTX_PUSH | G_MTX_MUL);
    gSPDisplayList(renderState->dl++, Space_SetTextures2);
    gSPDisplayList(renderState->dl++, cutscene_console_model_gfx);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

    int alpha = (int)(cutscene->currentFade * 255.0f);

    if (alpha > 255) {
        alpha = 255;
    } else if (alpha < 0) {
        alpha = 0;
    }

    gDPPipeSync(renderState->dl++);
    gDPSetRenderMode(renderState->dl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPPipeSync(renderState->dl++);
    gDPSetCombineLERP(renderState->dl++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
    gDPSetEnvColor(renderState->dl++, 0, 0, 0, 255 - alpha);
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD, SCREEN_HT);
    gDPPipeSync(renderState->dl++);

    textBoxRender(&gTextBox, renderState);
    
    spriteFinish(renderState);
}