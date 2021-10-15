#ifndef _SCENE_MANAGEMENT_H
#define _SCENE_MANAGEMENT_H

#include "level_scene.h"

enum SceneState {
    SceneStateNone,
    SceneStateInLevel,
};

extern enum SceneState gSceneState;
extern struct LevelScene gCurrentLevel;

void loadLevelScene();

void sceneUpdate();

#endif