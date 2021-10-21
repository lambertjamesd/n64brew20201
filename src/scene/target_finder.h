#ifndef _TARGET_FINDER_H
#define _TARGET_FINDER_H

#include "minion.h"
#include "collision/dynamicscene.h"

struct LevelScene;

struct TargetFinder {
    unsigned minionIndex;
    struct DynamicSceneEntry* trigger;
};

void targetFinderInit(struct TargetFinder* finder, unsigned startIndex);
void targetFinderUpdate(struct TargetFinder* finder);

#endif