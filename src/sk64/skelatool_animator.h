#ifndef _SKELATOOL_ANIMATION_H
#define _SKELATOOL_ANIMATION_H

#include <ultra64.h>
#include "math/quaternion.h"
#include "math/transform.h"
#include "skelatool_clip.h"

struct SKU16Vector3 {
    short x;
    short y;
    short z;
};

enum SKBoneStateFlags {
    SKBoneStateFlagsConstantPosition = (1 << 0),
    SKBoneStateFlagsConstantRotation = (1 << 1),
    SKBoneStateFlagsConstantScale = (1 << 2),
};

struct SKBoneState {
    unsigned short positionTick;
    unsigned short rotationTick;
    unsigned short scaleTick;
    unsigned short flags;
    struct SKU16Vector3 position;
    struct SKU16Vector3 scale;
    struct Quaternion rotation;
};

struct SKBoneAnimationState {
    struct SKBoneState prevState;
    struct SKBoneState nextState;
};

enum SKAnimatorFlags {
    SKAnimatorFlagsActive = (1 << 0),
    SKAnimatorFlagsLoop = (1 << 1),
    SKAnimatorFlagsPendingRequest = (1 << 2),
};

struct SKAnimator {
    unsigned short flags;
    unsigned short boneCount;
    float currentTime;
    unsigned short currTick;
    unsigned short nextTick;
    unsigned short nextSourceTick;
    unsigned short nextSourceChunkSize;
    u32 nextChunkSource;
    struct SKBoneAnimationState* boneState;
    struct SKAnimationHeader* currentAnimation;
};

#define SK_POOL_SIZE        (2 * 1024)
#define SK_POOL_QUEUE_SIZE  20

struct SKRingMemory {
    char __attribute__((aligned(16))) memoryStart[SK_POOL_SIZE];
    char* freeStart;
    char* usedStart;
    unsigned memoryUsed;
    unsigned memoryWasted;
};

struct SKAnimationDataPool {
    OSPiHandle* handle;
    OSMesgQueue mesgQueue;
    OSMesg mesgBuffer[SK_POOL_QUEUE_SIZE];
    OSIoMesg ioMessages[SK_POOL_QUEUE_SIZE];
    struct SKAnimator* animatorsForMessages[SK_POOL_QUEUE_SIZE];
    int nextMessage;
    struct SKRingMemory memoryPool;
};

void skInitDataPool(OSPiHandle* handle);
void skReadMessages();

void skAnimatorInit(struct SKAnimator* animator, unsigned boneCount);
void skAnimatorCleanup(struct SKAnimator* animator);
void skAnimatorRunClip(struct SKAnimator* animator, struct SKAnimationHeader* animation, int flags);
void skAnimatorUpdate(struct SKAnimator* animator, struct Transform* transforms, float timeScale);

void skAnimationApply(struct SKAnimator* animator, struct Transform* transforms, float tick);

#endif