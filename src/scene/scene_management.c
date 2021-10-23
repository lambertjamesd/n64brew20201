#include "scene_management.h"
#include "levels/level_test.h"

enum SceneState gSceneState;
struct LevelScene gCurrentLevel;

struct LevelDefinition* gLevels[] = {
    &gLevelTest,
};

void loadLevelScene() {
    levelSceneInit(&gCurrentLevel, gLevels[0], 4, 1);
    gSceneState = SceneStateInLevel;
}


void sceneUpdate() {
    switch (gSceneState) {
        case SceneStateInLevel:
            levelSceneUpdate(&gCurrentLevel);
            break;
    }
}