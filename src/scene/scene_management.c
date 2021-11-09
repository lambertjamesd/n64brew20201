#include "scene_management.h"
#include "levels/level_test.h"
#include "levels/levels.h"
#include "graphics/gfx.h"
#include "util/rom.h"

enum SceneState gSceneState;
struct LevelScene gCurrentLevel;

struct LevelDefinition* gLevelsTmp[] = {
    &gLevelTest,
};

extern char _staticSegmentRomStart[], _staticSegmentRomEnd[];

void loadLevelScene(struct LevelMetadata* metadata) {
    LOAD_SEGMENT(static, gStaticSegment);
    LOAD_SEGMENT(menu, gMenuSegment);
    LOAD_SEGMENT(characters, gCharacterSegment);

    if (gLevelSegment) {
        free(gLevelSegment);
        gLevelSegment = 0;
    }

    if (gThemeSegment) {
        free(gThemeSegment);
        gThemeSegment = 0;
    }

    gLevelSegment = malloc(metadata->romSegmentEnd - metadata->romSegmentStart);
    romCopy(metadata->romSegmentStart, gLevelSegment, metadata->romSegmentEnd - metadata->romSegmentStart);

    gThemeSegment = malloc(metadata->theme->romSegmentEnd - metadata->theme->romSegmentStart);
    romCopy(metadata->theme->romSegmentStart, gThemeSegment, metadata->theme->romSegmentEnd - metadata->theme->romSegmentStart);

    struct LevelDefinition* definition = levelDefinitionUnpack(metadata->fullDefinition, gLevelSegment, gThemeSegment);

    levelSceneInit(&gCurrentLevel, definition, 2, 1);
    gSceneState = SceneStateInLevel;
}


void sceneUpdate() {
    switch (gSceneState) {
        case SceneStateInLevel:
            levelSceneUpdate(&gCurrentLevel);
            break;
        default:
            break;
    }
}