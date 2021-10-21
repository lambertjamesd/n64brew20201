#ifndef _LEVEL_SCENE_H
#define _LEVEL_SCENE_H

#include <ultra64.h>

#include "game_defs.h"
#include "camera.h"
#include "minion.h"
#include "levelbase.h"
#include "graphics/render_state.h"
#include "leveldefinition.h"
#include "player.h"
#include "target_finder.h"

#define MAX_MINIONS_PER_BASE       3
#define TARGET_FINDER_COUNT        2

struct LevelScene {
    struct Camera cameras[MAX_PLAYERS];
    struct Player players[MAX_PLAYERS];
    unsigned char playerCount;
    unsigned char humanPlayerCount;

    struct LevelBase *bases;
    unsigned int baseCount;
    
    Gfx* levelDL;
    struct Minion* minions;
    unsigned int minionCount;
    unsigned lastMinion;

    struct TargetFinder targetFinders[TARGET_FINDER_COUNT];
};

void levelSceneInit(struct LevelScene* levelScene, struct LevelDefinition* definition, unsigned int playercount, unsigned char humanPlayerCount);
void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState);
void levelSceneUpdate(struct LevelScene* levelScene);

void levelSceneSpawnMinion(struct LevelScene* levelScene, enum MinionType type, struct Transform* at, unsigned char baseId, unsigned team, enum MinionCommand defualtCommand);
void levelBaseDespawnMinions(struct LevelScene* levelScene, unsigned char baseId);
void levelSceneIssueMinionCommand(struct LevelScene* levelScene, unsigned team, enum MinionCommand command);

#endif