#ifndef _COLLISION_DYNAMIC_SCENE
#define _COLLISION_DYNAMIC_SCENE

#include "shape.h"
#include "math/box2d.h"
#include "math/vector3.h"
#include "math/quaternion.h"

enum DynamicSceneEntryFlags {
    DynamicSceneEntryIsTrigger = (1 << 0),
    DynamicSceneEntryDirtyBox = (1 << 1),
    DynamicSceneEntryHasTeam = (1 << 8),
};

struct DynamicSceneOverlap;

typedef void (*CollisionCallback)(struct DynamicSceneOverlap* overlap);

struct DynamicSceneEntry {
    void* data;
    struct CollisionShape* forShape;
    struct Vector2 center;
    struct Vector2 rotation;
    struct Box2D boundingBox;
    CollisionCallback onCollide;
    unsigned short flags;
    unsigned short collisionLayers;
};

struct DynamicSceneOverlap {
    struct ShapeOverlap shapeOverlap;
    struct DynamicSceneEntry* thisEntry;
    struct DynamicSceneEntry* otherEntry;
};

#define DYNAMIC_SCENE_ENTRY_COUNT   128

struct DynamicScene {
    struct DynamicSceneEntry entries[DYNAMIC_SCENE_ENTRY_COUNT];
    struct DynamicSceneEntry* entryOrder[DYNAMIC_SCENE_ENTRY_COUNT];
    // TODO move this to the stack
    struct DynamicSceneEntry* workingMemory[DYNAMIC_SCENE_ENTRY_COUNT];
    unsigned short actorCount;
    unsigned short locked;
};

extern struct DynamicScene gDynamicScene;

void dynamicSceneInit(struct DynamicScene* scene);

struct DynamicSceneEntry* dynamicSceneNewEntry(
    struct CollisionShape* forShape, 
    void* data,
    struct Vector2* at,
    CollisionCallback onCollide,
    unsigned flags,
    unsigned collisionLayers
);
void dynamicSceneDeleteEntry(struct DynamicSceneEntry* entry);

void dynamicSceneCollide();

void dynamicEntrySetPos(struct DynamicSceneEntry* entry, struct Vector2* pos);
void dynamicEntrySetPos3D(struct DynamicSceneEntry* entry, struct Vector3* pos);
void dynamicEntrySetRotation3D(struct DynamicSceneEntry* entry, struct Quaternion* rotation);

#endif