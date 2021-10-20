
#include "dynamicscene.h"

#include "util/memory.h"

struct DynamicScene gDynamicScene;

void dynamicSceneInit(struct DynamicScene* scene) {
    zeroMemory(scene, sizeof(struct DynamicScene));

    for (unsigned i = 0; i < DYNAMIC_SCENE_ENTRY_COUNT; ++i) {
        gDynamicScene.entryOrder[i] = &gDynamicScene.entries[i];
    }
}

struct DynamicSceneEntry* dynamicSceneNewEntry(
    struct CollisionShape* forShape, 
    void* data,
    struct Vector2* at,
    CollisionCallback onCollide,
    unsigned flags,
    unsigned collisionLayers
) {
    if (gDynamicScene.actorCount < DYNAMIC_SCENE_ENTRY_COUNT) {
        struct DynamicSceneEntry* result = gDynamicScene.entryOrder[gDynamicScene.actorCount];
        result->forShape = forShape;
        result->data = data;
        result->center = *at;
        result->onCollide = onCollide;
        result->flags = flags;
        result->collisionLayers = collisionLayers;
        ++gDynamicScene.actorCount;
        return result;
    }

    return 0;
}

void dynamicSceneDeleteEntry(struct DynamicSceneEntry* entry) {
    int found = 0;

    for (unsigned i = 0; i < gDynamicScene.actorCount; ++i) {
        if (gDynamicScene.entryOrder[i] == entry) {
            found = 0;
            entry->forShape = 0;
        }

        if (found) {
            gDynamicScene.entryOrder[i] = gDynamicScene.entryOrder[i + 1];
        }
    }

    if (found) {
        --gDynamicScene.actorCount;
        gDynamicScene.entryOrder[gDynamicScene.actorCount] = entry;
    }
}

void dynamicSortEntries(struct DynamicSceneEntry** start, struct DynamicSceneEntry** end, struct DynamicSceneEntry** workingMemory) {
    if (start + 1 >= end) {
        return;
    } 

    if (start + 2 == end) {
        if ((*start)->boundingBox.min.x > (*(start+1))->boundingBox.min.x) {
            struct DynamicSceneEntry* tmp = *start;
            *start = *(start + 1);
            *(start + 1) = tmp;
            return;
        }
    }

    struct DynamicSceneEntry* slidingMerge;

    unsigned midOffset = (end - start) / 2;

    struct DynamicSceneEntry** mid = start + midOffset;
    dynamicSortEntries(start, mid, workingMemory);
    dynamicSortEntries(mid, end, workingMemory);

    struct DynamicSceneEntry** startIt = start;
    struct DynamicSceneEntry** midIt = mid;

    struct DynamicSceneEntry** outputIt = workingMemory;

    while (startIt < mid || midIt < end) {
        if (midIt == end || startIt < mid && (*startIt)->boundingBox.min.x < (*midIt)->boundingBox.min.x) {
            *outputIt = *startIt;
            ++startIt;
        } else {
            *outputIt = *midIt;
            ++midIt;
        }
        ++outputIt;
    }

    startIt = start;
    outputIt = workingMemory;

    while (startIt < end) {
        *startIt++ = *outputIt++;
    }
}

void dynamicSceneCheckCollision(struct DynamicSceneEntry* a, struct DynamicSceneEntry* b) {
    struct DynamicSceneOverlap overlap;
    struct Vector2 offset;
    vector2Sub(&b->center, &a->center, &offset);

    if (collisionCollidePair(a->forShape, b->forShape, &offset, ((a->flags | b->flags) & DynamicSceneEntryIsTrigger) ? 0 : &overlap.shapeOverlap)) {
        if (a->onCollide && !(b->flags & DynamicSceneEntryIsTrigger)) {
            overlap.thisEntry = a;
            overlap.otherEntry = b;
            a->onCollide(&overlap);
        }

        if (b->onCollide && !(a->flags & DynamicSceneEntryIsTrigger)) {
            overlap.shapeOverlap.depth = -overlap.shapeOverlap.depth;
            overlap.thisEntry = b;
            overlap.otherEntry = a;
            b->onCollide(&overlap);
        }
    }
}

void dynamicSceneCollide() {
    // update bounding boxes
    for (unsigned i = 0; i < gDynamicScene.actorCount; ++i) {
        collisionShapeBoundingBox(gDynamicScene.entryOrder[i]->forShape, &gDynamicScene.entryOrder[i]->center, &gDynamicScene.entryOrder[i]->boundingBox);
    }

    // sort by bounding box min x
    dynamicSortEntries(gDynamicScene.entryOrder, gDynamicScene.entryOrder + gDynamicScene.actorCount, gDynamicScene.workingMemory);

    unsigned activeEntries = 0;

    // collide overlapping entries
    for (unsigned i = 0; i < gDynamicScene.actorCount; ++i) {
        struct DynamicSceneEntry* currentEntry = gDynamicScene.entryOrder[i];

        float currentStart = currentEntry->boundingBox.min.x;

        unsigned writeIndex = 0;

        for (unsigned otherShapeIndex = 0; otherShapeIndex < activeEntries; ++otherShapeIndex) {
            struct DynamicSceneEntry* otherObject = gDynamicScene.workingMemory[otherShapeIndex];

            if (writeIndex != otherShapeIndex) {
                gDynamicScene.workingMemory[writeIndex] = gDynamicScene.workingMemory[otherShapeIndex];
            }

            if (otherObject->boundingBox.max.x >= currentStart) {
                ++writeIndex;

                if (currentEntry != otherObject &&
                    // colliders need to share at least one collision layer
                    (currentEntry->collisionLayers & otherObject->collisionLayers) != 0 &&
                    // if both colliders are triggers, ignore
                    ((currentEntry->flags & otherObject->flags & DynamicSceneEntryIsTrigger) == 0) &&
                    currentEntry->boundingBox.max.y >= otherObject->boundingBox.min.y && 
                    currentEntry->boundingBox.min.y <= otherObject->boundingBox.max.y &&
                    (currentEntry->onCollide != 0 || otherObject->onCollide != 0)) {
                    dynamicSceneCheckCollision(currentEntry, otherObject);
                }
            }
        }

        gDynamicScene.workingMemory[writeIndex] = currentEntry;
        activeEntries = writeIndex + 1;
    }
}