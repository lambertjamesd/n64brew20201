#include "menucommon.h"

#include "util/time.h"
#include "game_defs.h"
#include "scene/team_data.h"
#include "math/mathf.h"

#define CYCLE_TIME              2.0f

struct Coloru8 gDeselectedColor = {128, 128, 128, 255};

void menuSelectionColor(struct Coloru8* result) {
    float timeLerp = mathfMod(gTimePassed, CYCLE_TIME) * (MAX_PLAYERS / CYCLE_TIME);
    int colorIndex = ((int)timeLerp) % MAX_PLAYERS;
    int nextColor = (colorIndex + 1) % MAX_PLAYERS;
    colorU8Lerp(&gTeamColors[colorIndex], &gTeamColors[nextColor], timeLerp - colorIndex, result);
    colorU8Lerp(result, &gColorWhite, 0.5f, result);
}