#ifndef _SCENE_MANAGEMENT_H
#define _SCENE_MANAGEMENT_H

#include "level_scene.h"
#include "levels/levels.h"
#include "menu/mainmenu.h"
#include "graphics/render_state.h"

enum SceneState {
    SceneStateNone,
    SceneStateInLevel,
    SceneStateInMainMenu,
};

extern enum SceneState gSceneState;
extern struct LevelScene gCurrentLevel;
extern struct MainMenu gMainMenu;
extern char     _heapStart[];

int sceneIsLoading();

void sceneLoadLevel(struct LevelMetadata* metadata);
void sceneQueueLoadLevel(struct LevelMetadata* nextLevel);
void sceneQueueMainMenu();

void sceneUpdate(int hasActiveGraphics);
void sceneRender(struct RenderState* renderState);

#endif