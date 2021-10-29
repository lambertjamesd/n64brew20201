
#include <string.h>

#include "playerstatusmenu.h"
#include "scene/leveldefinition.h"
#include "graphics/spritefont.h"
#include "gbfont.h"
#include "graphics/gfx.h"

void playerStatusMenuRender(struct Player* player, struct RenderState* renderState, int winningTeam, unsigned short* screenPos) {
    char message[20];
    message[0] = '\0';
    int scaleShift = 0;
    int barHeight = 64;

    if (winningTeam != TEAM_NONE) {
        if (player->team.teamNumber == winningTeam) {
            strcpy(message, "Victory");
        } else {
            strcpy(message, "Defeat");
        }

        scaleShift = 1;
    } else if (!playerIsAlive(player)) {
        int respawnIn = (int)(player->stateTimer + 1.0f);
        sprintf(message, "Respawn in %d", respawnIn);
        barHeight = screenPos[3] - screenPos[1];
    }

    if (*message) {
        unsigned horizontalCenter = (screenPos[0] + screenPos[2]) >> 1;
        unsigned verticalCenter = (screenPos[1] + screenPos[3]) >> 1;

        spriteSetColor(renderState, LAYER_SOLID_COLOR, 0, 0, 0, 128);
        spriteSolid(
            renderState,
            LAYER_SOLID_COLOR,
            screenPos[0],
            verticalCenter - (barHeight >> 1),
            screenPos[2] - screenPos[0],
            barHeight
        );

        int messageWidth = fontMeasure(&gGBFont, message, scaleShift);
        fontRenderText(renderState, &gGBFont, message, horizontalCenter - (messageWidth >> 1), verticalCenter - (8 << scaleShift), scaleShift);        
    }
}