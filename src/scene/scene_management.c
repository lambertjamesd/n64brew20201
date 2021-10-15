#include "scene_management.h"

enum SceneState gSceneState;
struct LevelScene gCurrentLevel;

void loadLevelScene() {
    levelSceneInit(&gCurrentLevel, 1, 1);
    gSceneState = SceneStateInLevel;
}


void sceneUpdate() {
    switch (gSceneState) {
        case SceneStateInLevel:
            levelSceneUpdate(&gCurrentLevel);
            break;
    }
}