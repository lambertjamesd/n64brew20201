#ifndef _PLAYER_STATUS_MENU
#define _PLAYER_STATUS_MENU

#include "graphics/render_state.h"
#include "scene/player.h"

void playerStatusMenuRender(struct Player* player, struct RenderState* renderState, int winningTeam, float knockoutTime, unsigned short* screenPos);

#endif