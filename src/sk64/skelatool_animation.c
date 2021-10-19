
#include "skelatool_animation.h"
#include "util/memory.h"
#include "util/time.h"

#define TICK_UNDEFINED      ~((u16)(0))

static struct SKAnimationDataPool gSKAnimationPool;

void skRingMemoryInit(struct SKRingMemory* memory) {
    memory->freeStart = memory->memoryStart;
    memory->usedStart = memory->memoryStart;
    memory->memoryUsed = 0;
    memory->memoryWasted = 0;
}

void skRingMemoryCleanup(struct SKRingMemory* memory) {
    free(memory->memoryStart);
}

void* skRingMemoryAlloc(struct SKRingMemory* memory, int size) {
    int available = SK_POOL_SIZE - memory->memoryUsed - memory->memoryWasted;

    if (available < size) {
        return 0;
    }

    available = memory->memoryStart + SK_POOL_SIZE - memory->freeStart;

    if (available >= size) {
        char* result = memory->freeStart;
        memory->memoryUsed += size;
        memory->freeStart += size;
        return result;
    }

    int availableAtFront = memory->freeStart - memory->memoryStart;

    if (availableAtFront >= size) {
        char* result = memory->memoryStart;
        memory->memoryUsed += size;
        memory->memoryWasted = available;
        memory->freeStart = result + size;
        return result;
    }

    return 0;
}

void skRingMemoryFree(struct SKRingMemory* memory, int size) {
    memory->usedStart += size;
    memory->memoryUsed -= size;

    if (memory->usedStart + memory->memoryWasted >= memory->memoryStart + SK_POOL_SIZE) {
        memory->usedStart = memory->memoryStart;
        memory->memoryWasted = 0;
    }
}

struct SKBoneKeyframe* skApplyBoneKeyframe(struct SKAnimator* animator, struct SKBoneKeyframe* keyframe, u16 tick) {   
    short* attrInput = &keyframe->attributeData[0];

    struct SKBoneAnimationState* boneState = &animator->boneState[keyframe->boneIndex];

    if (keyframe->usedAttributes & SKBoneAttrMaskPosition) {
        boneState->prevState.positionTick = boneState->nextState.positionTick;
        boneState->prevState.position = boneState->nextState.position;

        boneState->nextState.positionTick = tick;
        boneState->nextState.position.x = *attrInput++;
        boneState->nextState.position.y = *attrInput++;
        boneState->nextState.position.z = *attrInput++;
    }

    if (keyframe->usedAttributes & SKBoneAttrMaskRotation) {
        boneState->prevState.rotationTick = boneState->nextState.rotationTick;
        boneState->prevState.rotation = boneState->nextState.rotation;

        boneState->nextState.rotationTick = tick;
        boneState->nextState.rotation.x = (*attrInput++) / 32767.0f;
        boneState->nextState.rotation.y = (*attrInput++) / 32767.0f;
        boneState->nextState.rotation.z = (*attrInput++) / 32767.0f;
        float wSqrd = 1.0f - (boneState->nextState.rotation.x * boneState->nextState.rotation.x + boneState->nextState.rotation.y * boneState->nextState.rotation.y + boneState->nextState.rotation.z * boneState->nextState.rotation.z);

        if (wSqrd <= 0.0f) {
            boneState->nextState.rotation.w = 0.0f;
        } else {
            boneState->nextState.rotation.w = sqrtf(wSqrd);
        }
    }

    if (keyframe->usedAttributes & SKBoneAttrMaskScale) {
        boneState->prevState.scaleTick = boneState->nextState.scaleTick;
        boneState->prevState.scale = boneState->nextState.scale;

        boneState->nextState.scaleTick = tick;
        boneState->nextState.scale.x = *attrInput++;
        boneState->nextState.scale.y = *attrInput++;
        boneState->nextState.scale.z = *attrInput++;
    }

    return (struct SKBoneKeyframe*)attrInput;
}

struct SKAnimationKeyframe* skApplyKeyframe(struct SKAnimator* animator, struct SKAnimationKeyframe* keyframe) {
    struct SKBoneKeyframe* currentBone = &keyframe->bones[0];

    for (unsigned i = 0; i < keyframe->boneCount; ++i) {
        currentBone = skApplyBoneKeyframe(animator, currentBone, keyframe->tick);
    }

    return (struct SKAnimationKeyframe*)currentBone;
}


void skApplyChunk(struct SKAnimator* animator, struct SKAnimationChunk* chunk) {
    struct SKAnimationKeyframe* nextFrame = &chunk->keyframes[0];

    for (unsigned i = 0; i < chunk->keyframeCount; ++i) {
        nextFrame = skApplyKeyframe(animator, nextFrame);
    }
}

void skProcess(OSIoMesg* message) {
    int messageIndex = message - gSKAnimationPool.ioMessages;

    struct SKAnimator* animator = gSKAnimationPool.animatorsForMessages[messageIndex];

    if (animator) {
        gSKAnimationPool.animatorsForMessages[messageIndex] = 0;

        struct SKAnimationChunk* nextChunk = (struct SKAnimationChunk*)message->dramAddr;

        skApplyChunk(animator, nextChunk);
        animator->flags &= ~SKAnimatorFlagsPendingRequest;
        animator->nextChunkSource += animator->nextSourceChunkSize;
        skRingMemoryFree(&gSKAnimationPool.memoryPool, animator->nextSourceChunkSize);
        animator->nextSourceTick = nextChunk->nextChunkTick;
        animator->nextSourceChunkSize = nextChunk->nextChunkSize;
    }
}

void skInitDataPool(OSPiHandle* handle) {
    gSKAnimationPool.handle = handle;
    gSKAnimationPool.nextMessage = 0;
    osCreateMesgQueue(&gSKAnimationPool.mesgQueue, gSKAnimationPool.mesgBuffer, SK_POOL_QUEUE_SIZE);
    skRingMemoryInit(&gSKAnimationPool.memoryPool);
    zeroMemory(gSKAnimationPool.animatorsForMessages, sizeof(struct SKAnimator*) * SK_POOL_QUEUE_SIZE);
}

void skReadMessages() {
    OSMesg msg;
    while (osRecvMesg(&gSKAnimationPool.mesgQueue, &msg, OS_MESG_NOBLOCK) != -1) {
        skProcess(msg);
    }
}

void skelatoolWaitForNextMessage() {
    OSMesg msg;
    osRecvMesg(&gSKAnimationPool.mesgQueue, &msg, OS_MESG_BLOCK);
    skProcess(msg);
}

char tmp[1024];

void skRequestChunk(struct SKAnimator* animator) {
    unsigned short chunkSize = animator->nextSourceChunkSize;

    if (chunkSize == 0) {
        return;
    }

    if (chunkSize > SK_POOL_SIZE || !animator->nextChunkSource) {
        // chunk can't possitbly fit exit early
        return;
    }

    unsigned short retries = 0;

    char* dest = skRingMemoryAlloc(&gSKAnimationPool.memoryPool, chunkSize);

    // wait until enough memory is avaialble
    while (!dest || gSKAnimationPool.mesgQueue.validCount == gSKAnimationPool.mesgQueue.msgCount) {
        // something is wrong, avoid an infinite loop
        if (retries == SK_POOL_QUEUE_SIZE) {
            animator->flags &= ~SKAnimatorFlagsActive;
            return;
        }
        skelatoolWaitForNextMessage();
        dest = skRingMemoryAlloc(&gSKAnimationPool.memoryPool, chunkSize);
        ++retries;
    }

    animator->flags |= SKAnimatorFlagsPendingRequest;

    // request new chunk
    OSIoMesg* ioMesg = &gSKAnimationPool.ioMessages[gSKAnimationPool.nextMessage];
    gSKAnimationPool.animatorsForMessages[gSKAnimationPool.nextMessage] = animator;
    gSKAnimationPool.nextMessage = (gSKAnimationPool.nextMessage + 1) % SK_POOL_QUEUE_SIZE;

    ioMesg->hdr.pri = OS_MESG_PRI_NORMAL;
    ioMesg->hdr.retQueue = &gSKAnimationPool.mesgQueue;
    ioMesg->dramAddr = (void*)dest;
    ioMesg->devAddr = (u32)animator->nextChunkSource;
    ioMesg->size = chunkSize;

    osEPiStartDma(gSKAnimationPool.handle, ioMesg, OS_READ);
}

void skFixedVector3ToFloat(struct SKU16Vector3* input, struct Vector3* output) {
    output->x = (float)input->x;
    output->y = (float)input->y;
    output->z = (float)input->z;
}

void skApplyBoneAnimation(struct SKBoneAnimationState* animatedBone, struct Transform* output, float tick) {
    if (animatedBone->nextState.positionTick != animatedBone->prevState.positionTick) {
        float positionLerp = (tick - (float)animatedBone->prevState.positionTick) / ((float)animatedBone->nextState.positionTick - (float)animatedBone->prevState.positionTick);
        struct Vector3 srcPos;
        skFixedVector3ToFloat(&animatedBone->prevState.position, &srcPos);
        skFixedVector3ToFloat(&animatedBone->nextState.position, &output->position);
        vector3Lerp(&srcPos, &output->position, positionLerp, &output->position);
    } else {
        skFixedVector3ToFloat(&animatedBone->nextState.position, &output->position);
    }

    if (animatedBone->nextState.rotationTick != animatedBone->prevState.rotationTick) {
        float rotationLerp = (tick - (float)animatedBone->prevState.rotationTick) / ((float)animatedBone->nextState.rotationTick - (float)animatedBone->prevState.rotationTick);
        quatLerp(&animatedBone->prevState.rotation, &animatedBone->nextState.rotation, rotationLerp, &output->rotation);
    } else {
        output->rotation = animatedBone->nextState.rotation;
    }

    if (animatedBone->nextState.scaleTick != animatedBone->prevState.scaleTick) {
        float scaleLerp = (tick - (float)animatedBone->prevState.scaleTick) / ((float)animatedBone->nextState.scaleTick - (float)animatedBone->prevState.scaleTick);
        struct Vector3 srcScale;
        skFixedVector3ToFloat(&animatedBone->prevState.scale, &srcScale);
        skFixedVector3ToFloat(&animatedBone->nextState.scale, &output->scale);
        vector3Lerp(&srcScale, &output->scale, scaleLerp, &output->scale);
        vector3Scale(&output->scale, &output->scale, 1.0f / 256.0f);
    } else {
        skFixedVector3ToFloat(&animatedBone->nextState.scale, &output->scale);
        vector3Scale(&output->scale, &output->scale, 1.0f / 256.0f);
    }
}

void skAnimatorInit(struct SKAnimator* animator, unsigned boneCount) {
    animator->flags = 0;
    animator->boneCount = boneCount;
    animator->currentTime = 0.0f;
    animator->currTick = TICK_UNDEFINED;
    animator->nextTick = 0;
    animator->nextSourceTick = TICK_UNDEFINED;
    animator->nextSourceChunkSize = 0;
    animator->nextChunkSource = 0;
    animator->boneState = malloc(sizeof(struct SKBoneAnimationState) * boneCount); 
    animator->currentAnimation = 0;
}

void skAnimatorCleanup(struct SKAnimator* animator) {
    free(animator->boneState);
    animator->boneState = 0;
    animator->flags = 0;
    animator->currentAnimation = 0;
}

void skAnimatorRunClip(struct SKAnimator* animator, struct SKAnimationHeader* animation, int flags) {
    animator->flags = (unsigned short)(flags | SKAnimatorFlagsActive);
    animator->currentTime = 0.0f;
    animator->currTick = TICK_UNDEFINED;
    animator->nextTick = 0;
    animator->nextSourceTick = TICK_UNDEFINED;
    animator->nextSourceChunkSize = animation->firstChunkSize;
    animator->nextChunkSource = (u32)animation->firstChunk;
    animator->currentAnimation = animation;

    skRequestChunk(animator);
}

void skAnimationApply(struct SKAnimator* animator, struct Transform* transforms, float tick) {
    for (unsigned i = 0; i < animator->boneCount; ++i) {
        skApplyBoneAnimation(&animator->boneState[i], &transforms[i], tick);
    }
}

void skAnimatorUpdate(struct SKAnimator* animator, struct SKArmature* object, float timeScale) {
    if (!(animator->flags & SKAnimatorFlagsActive) || !animator->currentAnimation) {
        return;
    }
    
    animator->currTick = animator->nextTick;
    float currTick = animator->currentTime * animator->currentAnimation->ticksPerSecond;
    animator->currentTime += gTimeDelta * timeScale;
    animator->nextTick = (u16)(animator->currentTime * animator->currentAnimation->ticksPerSecond);

    if (animator->currTick <= animator->currentAnimation->maxTicks && object) {
        skAnimationApply(animator, object->boneTransforms, currTick);
    }

    if (animator->flags & SKAnimatorFlagsLoop) {
        if (animator->nextTick >= animator->currentAnimation->maxTicks) {
            skAnimatorRunClip(animator, animator->currentAnimation, animator->flags);
            return;
        }
    }
    
    // queue up next keyframes if they are needed
    if (animator->nextTick >= animator->nextSourceTick) {
        animator->nextSourceTick = TICK_UNDEFINED;
        skRequestChunk(animator);
    }
}