#include "titlescreen.h"

#include "../data/mainmenu/menu.h"
#include "math/mathf.h"
#include "util/time.h"
#include "graphics/image.h"
#include "graphics/gfx.h"
#include "controls/controller.h"

#define TITLE_SCROLL_TIME     0.5f

#define MISSION_X       22
#define MISSION_Y       21

#define LOST_CONTROL_X  68
#define LOST_CONTROL_Y  53

#define START_X         96
#define START_Y         151

#define IMAGE_X         23
#define IMAGE_Y         66

void titleScreenInit(struct TitleScreen* titleScreen) {
    titleScreen->timer = TITLE_SCROLL_TIME;
    titleScreen->targetTime = 0.0f;
}

int titleScreenUpdate(struct TitleScreen* titleScreen) {
    titleScreen->timer = mathfMoveTowards(titleScreen->timer, titleScreen->targetTime, gTimeDelta);
    if (titleScreen->targetTime == TITLE_SCROLL_TIME && titleScreen->timer == TITLE_SCROLL_TIME) {
        return 1;
    }

    if (controllerGetButtonDown(0, START_BUTTON)) {
        titleScreen->targetTime = TITLE_SCROLL_TIME;
    }

    return 0;
}

void titleScreenRender(struct TitleScreen* titleScreen, struct RenderState* renderState) {
    graphicsCopyImage(
        renderState,
        TitleScreenImage_0_0,
        256,
        174,
        0,
        0,
        IMAGE_X,
        IMAGE_Y,
        256,
        174,
        gColorWhite
    );

    graphicsCopyImage(
        renderState, 
        TitleMission_0_0, 
        162, 
        32, 
        0, 
        0, 
        MISSION_X + (int)((SCREEN_WD / TITLE_SCROLL_TIME) * titleScreen->timer), 
        MISSION_Y, 
        162, 
        31, 
        gColorWhite
    );

    graphicsCopyImage(
        renderState, 
        TitleLostcontrol_0_0, 
        233, 
        18, 
        0, 
        0, 
        LOST_CONTROL_X - (int)((SCREEN_WD / TITLE_SCROLL_TIME) * titleScreen->timer), 
        LOST_CONTROL_Y, 
        233, 
        18, 
        gColorWhite
    );

    graphicsCopyImage(
        renderState, 
        Start_Button_0, 
        129, 
        44, 
        0, 
        0, 
        START_X, 
        START_Y + (int)((SCREEN_HT / TITLE_SCROLL_TIME) * titleScreen->timer), 
        129, 
        44, 
        gColorWhite
    );
}