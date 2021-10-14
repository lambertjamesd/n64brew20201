#include "scene_management.h"

struct LevelScene gCurrentLevel;

void loadLevelScene() {
    initLevelScene(&gCurrentLevel, 1);
}