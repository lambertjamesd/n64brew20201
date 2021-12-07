#include "spinninglogo.h"

#include "graphics/spritefont.h"
#include "../data/mainmenu/menu.h"
#include "../data/fonts/fonts.h"
#include "kickflipfont.h"
#include "game_defs.h"
#include "graphics/gfx.h"
#include "math/color.h"
#include "util/time.h"
#include "menu/fresheater.h"
#include "scene/scene_management.h"
#include "audio/audio.h"

#define WHITE_CLEAR_TIME        0.5f
#define DELAY_TIME              2.0f
#define SPINNING_LOGO_TIME      (DELAY_TIME + 7.0f)
// #define SPINNING_LOGO_TIME      2.5f
#define FADE_TIME               2.0f
#define JINGLE_PLAY_TIME        (DELAY_TIME)
#define LAUGH_PLAY_TIME         (DELAY_TIME + 3.0f)
#define RADS_PER_SEC            (M_PI*1/3)
#define RAD_OFFSET              (M_PI*0.5f)


struct SpinningLogo gSpinningLogo;

void spinningLogoInit(struct SpinningLogo* spinningLogo) {
    spinningLogo->timer = 0.0f;
    cameraInit(&spinningLogo->camera, 70.0f, 1.0f * 256.0f, 128.0f * 256.0f);

    spinningLogo->camera.transform.position.x = 0.0;
    spinningLogo->camera.transform.position.y = 2.0 * 256.0f;
    spinningLogo->camera.transform.position.z = 5.0 * 256.0f;

    quatAxisAngle(&gRight, -27.0f * (M_PI / 180.0f), &spinningLogo->camera.transform.rotation);

    spinningLogo->camera.transform.scale = gOneVec;
    
    initFreshEaterFont();
}

void spinningLogoUpdate(struct SpinningLogo* spinningLogo) {
    if (spinningLogo->timer > SPINNING_LOGO_TIME) {
        sceneQueueMainMenu();
        sceneInsertCutscene(CutsceneIndexIntro);
    } else {
        float nextTime = spinningLogo->timer + gTimeDelta;

        if (nextTime >= JINGLE_PLAY_TIME && spinningLogo->timer < JINGLE_PLAY_TIME) {
            playJingle();
        }

        if (nextTime >= LAUGH_PLAY_TIME && spinningLogo->timer < LAUGH_PLAY_TIME) {
            soundPlayerPlay(SOUNDS_LOGOLAUGHING, 0.7f, 0, 0);
        }

        spinningLogo->timer = nextTime;
    }
}

void spinningLogoRender(struct SpinningLogo* spinningLogo, struct RenderState* renderState) {
    spriteSetLayer(renderState, LAYER_SOLID_COLOR, gMainMenuSolidColor);
    spriteSetLayer(renderState, LAYER_KICKFLIP_NUMBERS_FONT, gUseKickflipNumbersFont);
    spriteSetLayer(renderState, LAYER_KICKFLIP_FONT, gUseKickflipFont);
    spriteSetLayer(renderState, LAYER_FRESH_EATERS, gFreshEaterUse);

    gDPSetFillColor(renderState->dl++, (GPACK_ZDZ(G_MAXFBZ,0) << 16 | GPACK_ZDZ(G_MAXFBZ,0)));
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    gDPSetTexturePersp(renderState->dl++, G_TP_PERSP);
    cameraSetupMatrices(&spinningLogo->camera, renderState, (float)SCREEN_WD/(float)SCREEN_HT, 0.0f);
    gSPViewport(renderState->dl++, osVirtualToPhysical(&gFullScreenVP));
    gDPSetScissor(
        renderState->dl++, 
        G_SC_NON_INTERLACE, 
        0,
        0,
        SCREEN_WD,
        SCREEN_HT
    );
    gDPPipeSync(renderState->dl++);
    gDPSetCycleType(renderState->dl++, G_CYC_1CYCLE);
    gDPSetRenderMode(renderState->dl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);

    struct Transform spinningTransform;
    spinningTransform.position = gZeroVec;
    vector3Scale(&gOneVec, &spinningTransform.scale, 1.0f);
    quatAxisAngle(
        &gUp, 
        -spinningLogo->timer * RADS_PER_SEC + RAD_OFFSET, 
        &spinningTransform.rotation
    );

    Mtx* nextTransfrom = renderStateRequestMatrices(renderState, 1);
    transformToMatrixL(&spinningTransform, nextTransfrom);
    gSPSetGeometryMode(renderState->dl++, G_ZBUFFER);
    gDPSetRenderMode(renderState->dl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    gSPMatrix(renderState->dl++, OS_K0_TO_PHYSICAL(nextTransfrom), G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_PUSH);
    gSPDisplayList(renderState->dl++, SpinningLogo_SpinningLogo_mesh);
    gSPPopMatrix(renderState->dl++, G_MTX_MODELVIEW);

    fontRenderText(
        renderState,
        &gFreshEater,
        "ULTRA RARE",
        80,
        SCREEN_HT / 2 + 43,
        0
    );

    spriteFinish(renderState);


    u8 alpha = 255;

    if (spinningLogo->timer < DELAY_TIME) {
        alpha = 0;
    } else if (spinningLogo->timer < DELAY_TIME + FADE_TIME) {
        alpha = (u8)(255 * (spinningLogo->timer - DELAY_TIME) / FADE_TIME);
    } else if (spinningLogo->timer >= SPINNING_LOGO_TIME) {
        alpha = 0;
    } else if (spinningLogo->timer > SPINNING_LOGO_TIME - FADE_TIME) {
        alpha = (u8)(255 * (SPINNING_LOGO_TIME - spinningLogo->timer) / FADE_TIME);
    }

    gDPPipeSync(renderState->dl++);
    gDPSetCombineLERP(renderState->dl++, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
    if (spinningLogo->timer < WHITE_CLEAR_TIME) {
        gDPSetEnvColor(renderState->dl++, 32, 32, 32, 255 - alpha);
    } else {
        gDPSetEnvColor(renderState->dl++, 0, 0, 0, 255 - alpha);
    }
    gDPFillRectangle(renderState->dl++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
    gDPPipeSync(renderState->dl++);
}