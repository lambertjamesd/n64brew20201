
#include <string.h>

#include "playerstatusmenu.h"
#include "scene/leveldefinition.h"
#include "graphics/spritefont.h"
#include "kickflipfont.h"
#include "graphics/gfx.h"

void playerStatusMenuRender(struct Player* player, struct RenderState* renderState, int winningTeam, float knockoutTime, unsigned short* screenPos) {
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

        scaleShift = 0;
    } else if (knockoutTime >= 0.0f && player->controlledBases) {
        sprintf(message, "Victory in %d", (int)(knockoutTime + 1));
        scaleShift = -1;
    } else if (knockoutTime >= 0.0f && !player->controlledBases) {
        sprintf(message, "Defeat in %d", (int)(knockoutTime + 1));
        scaleShift = -1;
    } else if (!playerIsAlive(player)) {
        if (player->controlledBases == 0) {
            strcpy(message, "Defeat");
            scaleShift = 0;
        } else {
            int respawnIn = (int)(player->stateTimer + 1.0f);
            sprintf(message, "Respawn in %d", respawnIn);
            scaleShift = -1;
            barHeight = screenPos[3] - screenPos[1];
        }
    }

    if (*message) {
        unsigned horizontalCenter = (screenPos[0] + screenPos[2]) >> 1;
        unsigned verticalCenter = (screenPos[1] + screenPos[3]) >> 1;

        spriteSetColor(renderState, LAYER_SOLID_COLOR, gHalfTransparentBlack);
        spriteSolid(
            renderState,
            LAYER_SOLID_COLOR,
            screenPos[0],
            verticalCenter - (barHeight >> 1),
            screenPos[2] - screenPos[0],
            barHeight
        );

        int messageWidth = fontMeasure(&gKickflipFont, message, scaleShift);
        fontRenderText(renderState, &gKickflipFont, message, horizontalCenter - (messageWidth >> 1), verticalCenter - (4 << (scaleShift + 1)), scaleShift);        
    }
}