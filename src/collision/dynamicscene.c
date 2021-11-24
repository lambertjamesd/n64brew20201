
#include "dynamicscene.h"

#include "util/memory.h"
#include <assert.h>

struct DynamicScene gDynamicScene;

void dynamicSceneInit(struct DynamicScene* scene, unsigned short actorCapacity) {
    zeroMemory(scene, sizeof(struct DynamicScene));
    scene->actorCapacity = actorCapacity;

    scene->entries = malloc(sizeof(struct DynamicSceneEntry) * actorCapacity);
    zeroMemory(scene->entries, sizeof(struct DynamicSceneEntry) * actorCapacity);

    scene->entryOrder = malloc(sizeof(struct DynamicSceneEntry*) * actorCapacity);
    scene->workingMemory = malloc(sizeof(struct DynamicSceneEntry*) * actorCapacity);

    for (unsigned i = 0; i < actorCapacity; ++i) {
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
    assert(!gDynamicScene.locked);
    
    if (!forShape) {
        return 0;
    }

    if (gDynamicScene.actorCount < gDynamicScene.actorCapacity) {
        struct DynamicSceneEntry* result = gDynamicScene.entryOrder[gDynamicScene.actorCount];
        result->forShape = forShape;
        result->data = data;
        result->center = *at;
        result->rotation.x = 1.0f;
        result->rotation.y = 0.0f;
        result->scale = 1.0f;
        result->onCollide = onCollide;
        result->flags = flags | DynamicSceneEntryDirtyBox;
        result->collisionLayers = collisionLayers;
        ++gDynamicScene.actorCount;
        return result;
    }

    return 0;
}

void dynamicSceneDeleteEntry(struct DynamicSceneEntry* entry) {
    assert(!gDynamicScene.locked);

    int found = 0;

    for (unsigned i = 0; i < gDynamicScene.actorCount; ++i) {
        if (gDynamicScene.entryOrder[i] == entry) {
            found = 1;
            entry->forShape = 0;
        }

        if (found) {
            gDynamicScene.entryOrder[i] = gDynamicScene.entryOrder[i + 1];
        }
    }

    assert(found);

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

    unsigned midOffset = (end - start) / 2;

    struct DynamicSceneEntry** mid = start + midOffset;
    dynamicSortEntries(start, mid, workingMemory);
    dynamicSortEntries(mid, end, workingMemory);

    struct DynamicSceneEntry** startIt = start;
    struct DynamicSceneEntry** midIt = mid;

    struct DynamicSceneEntry** outputIt = workingMemory;

    while (startIt < mid || midIt < end) {
        if (midIt == end || (startIt < mid && (*startIt)->boundingBox.min.x < (*midIt)->boundingBox.min.x)) {
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

    int shouldRotate = 0;
    int shouldScale = 0;

    if (a->forShape->type == CollisionShapeTypePolygon && (a->rotation.x != 1.0f || b->rotation.y != 0.0f)) {
        shouldRotate = 1;
        struct Vector2 invRotation;
        vector2ComplexConj(&a->rotation, &invRotation);
        vector2ComplexMul(&offset, &invRotation, &offset);
    } else if (b->forShape->type == CollisionShapeTypePolygon && (b->rotation.x != 1.0f || b->rotation.y != 0.0f)) {
        shouldRotate = 1;
        struct Vector2 invRotation;
        vector2ComplexConj(&b->rotation, &invRotation);
        vector2ComplexMul(&offset, &invRotation, &offset);
    }

    if (a->forShape->type == CollisionShapeTypePolygon && a->scale != 1.0f) {
        vector2Scale(&offset, 1.0f / a->scale, &offset);
        shouldRotate = 1;
    } else if (b->forShape->type == CollisionShapeTypePolygon && b->scale != 1.0f) {
        vector2Scale(&offset, 1.0f / b->scale, &offset);
        shouldRotate = 1;
    }

    if (collisionCollidePair(a->forShape, b->forShape, &offset, ((a->flags | b->flags) & DynamicSceneEntryIsTrigger) ? 0 : &overlap.shapeOverlap)) {
        if (shouldRotate) {
            if (a->forShape->type == CollisionShapeTypePolygon) {
                vector2ComplexMul(&overlap.shapeOverlap.normal, &a->rotation, &overlap.shapeOverlap.normal);
            } else {
                vector2ComplexMul(&overlap.shapeOverlap.normal, &b->rotation, &overlap.shapeOverlap.normal);
            }
        }

        if (shouldScale) {
            if (a->forShape->type == CollisionShapeTypePolygon) {
                overlap.shapeOverlap.depth *= a->scale;
            } else {
                overlap.shapeOverlap.depth *= b->scale;
            }
        }

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
        struct DynamicSceneEntry* currentEntry = gDynamicScene.entryOrder[i];
        
        if (currentEntry->flags & DynamicSceneEntryDirtyBox) {
            collisionShapeBoundingBox(currentEntry->forShape, &currentEntry->center, &currentEntry->rotation, currentEntry->scale, &currentEntry->boundingBox);
            currentEntry->flags &= ~DynamicSceneEntryDirtyBox;
        }
    }

    // sort by bounding box min x
    dynamicSortEntries(gDynamicScene.entryOrder, gDynamicScene.entryOrder + gDynamicScene.actorCount, gDynamicScene.workingMemory);

    unsigned activeEntries = 0;

    gDynamicScene.locked = 1;

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

    gDynamicScene.locked = 0;
}

void dynamicEntrySetPos(struct DynamicSceneEntry* entry, struct Vector2* pos) {
    entry->center = *pos;
    entry->flags |= DynamicSceneEntryDirtyBox;
}

void dynamicEntrySetPos3D(struct DynamicSceneEntry* entry, struct Vector3* pos) {
    entry->center.x = pos->x;
    entry->center.y = pos->z;
    entry->flags |= DynamicSceneEntryDirtyBox;
}

void dynamicEntrySetRotation3D(struct DynamicSceneEntry* entry, struct Quaternion* rotation) {
    struct Vector3 right;
    quatMultVector(rotation, &gRight, &right);
    right.y = 0.0f;
    vector3Normalize(&right, &right);
    entry->rotation.x = right.x;
    entry->rotation.y = right.z;
    entry->flags |= DynamicSceneEntryDirtyBox;
}

void dynamicEntrySetScale(struct DynamicSceneEntry* entry, float scale) {
    entry->scale = scale;
    entry->flags |= DynamicSceneEntryDirtyBox;
}