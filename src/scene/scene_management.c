#include "scene_management.h"

struct LevelScene gCurrentLevel;

void loadLevelScene() {
    levelSceneInit(&gCurrentLevel, 1, 1);
}