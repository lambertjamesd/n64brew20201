#ifndef _SKELATOOL_ANIMATION_H
#define _SKELATOOL_ANIMATION_H

#include <ultra64.h>
#include "math/quaternion.h"
#include "skelatool_object.h"
#include "skelatool_animation_clip.h"

struct SKU16Vector3 {
    unsigned short x;
    unsigned short y;
    unsigned short z;
};

struct SKBoneState {
    unsigned short positionTick;
    struct SKU16Vector3 position;
    unsigned short rotationTick;
    struct Quaternion rotation;
    unsigned short scaleTick;
    struct SKU16Vector3 scale;
};

struct SKBoneAnimationState {
    struct SKBoneState prevState;
    struct SKBoneState nextState;
};

enum SKAnimatorFlags {
    SKAnimatorFlagsActive = (1 << 0),
    SKAnimatorFlagsLoop = (1 << 1),
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

struct SKRingMemory {
    char* memoryStart;
    unsigned memorySize;
    char* freeStart;
    char* usedStart;
    unsigned memoryUsed;
    unsigned memoryWasted;
};

struct SKAnimationDataPool {
    OSPiHandle* handle;
    OSMesgQueue mesgQueue;
    OSMesg* mesgBuffer;
    OSIoMesg* ioMessages;
    struct SKAnimator** animatorsForMessages;
    int numMessages;
    int nextMessage;
    struct SKRingMemory memoryPool;
};

void skInitDataPool(int numMessages, int poolSize);
void skReadMessages();

void skAnimatorInit(struct SKAnimator* animator, unsigned boneCount);
void skAnimatorCleanup(struct SKAnimator* animator);
void skAnimatorRunClip(struct SKAnimator* animator, struct SKAnimationHeader* animation, int flags);
void skAnimatorUpdate(struct SKAnimator* animator, struct SKObject* object);

#endif