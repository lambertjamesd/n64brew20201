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
#include "menu/basecommandmenu.h"
#include "itemdrop.h"
#include "controlscrambler.h"
#include "ai/ai_controller.h"

#define MAX_MINIONS_PER_BASE       3
#define TARGET_FINDER_COUNT        2

enum LevelSceneState {
    LevelSceneStateIntro,
    LevelSceneStatePlaying,
    LevelSceneStateDone,
};

struct LevelScene {
    struct LevelDefinition* definition;
    struct Camera cameras[MAX_PLAYERS];
    struct Player players[MAX_PLAYERS];
    struct ControlsScrambler scramblers[MAX_PLAYERS];
    struct AIController* bots;
    struct BaseCommandMenu baseCommandMenu[MAX_PLAYERS];
    unsigned char playerCount;
    unsigned char humanPlayerCount;
    unsigned char botsCount;
    unsigned char winningTeam;

    unsigned short levelFlags;

    struct LevelBase *bases;
    unsigned int baseCount;

    Mtx* decorMatrices;
    
    Gfx* levelDL;
    struct Minion* minions;
    unsigned int minionCount;
    unsigned lastMinion;

    struct TargetFinder targetFinders[TARGET_FINDER_COUNT];

    struct ItemDrops itemDrops;

    enum LevelSceneState state;
    float stateTimer;
    float knockoutTimer;
};

struct Player* levelGetClosestEnemyPlayer(struct LevelScene* forScene, struct Vector3* closeTo, unsigned team, float* outDist);
struct Minion* levelGetClosestEnemyMinion(struct LevelScene* forScene, struct Vector3* closeTo, unsigned team, float* outDist);

void levelSceneInit(struct LevelScene* levelScene, struct LevelDefinition* definition, unsigned int playercount, unsigned char humanPlayerCount, enum LevelMetadataFlags flags);
void levelSceneRender(struct LevelScene* levelScene, struct RenderState* renderState);
void levelSceneUpdate(struct LevelScene* levelScene);

void levelSceneSpawnMinion(struct LevelScene* levelScene, enum MinionType type, struct Transform* at, unsigned char baseId, unsigned team, enum MinionCommand defualtCommand, unsigned followPlayer);
void levelBaseDespawnMinions(struct LevelScene* levelScene, unsigned char baseId);
void levelSceneIssueMinionCommand(struct LevelScene* levelScene, unsigned team, enum MinionCommand command);
struct Vector3* levelSceneFindRespawnPoint(struct LevelScene* levelScene, struct Vector3* closeTo, unsigned team);
int levelSceneFindWinningTeam(struct LevelScene* levelScene);

void levelSceneApplyScrambler(struct LevelScene* levelScene, unsigned fromTeam, enum ControlsScramblerType scambler);

#endif