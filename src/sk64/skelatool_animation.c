
#include "skelatool_animation.h"
#include "util/memory.h"
#include "util/time.h"

#define TICK_UNDEFINED      ~((u16)(0))

static struct SKAnimationDataPool gSKAnimationPool;

struct SKBoneKeyframe* skApplyBoneKeyframe(struct SKAnimator* animator, struct SKBoneKeyframe* keyframe, u16 tick, int* hasMore) {
    *hasMore = keyframe->usedAttributes != 0;
    if (!*hasMore) {
        return (struct SKBoneKeyframe*)&keyframe->attributeData[0];
    }
    
    unsigned short boneIndex = keyframe->usedAttributes >> 3;

    unsigned short* attrInput = &keyframe->attributeData[0];

    struct SKBoneAnimationState* boneState = &animator->boneState[boneIndex];

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

        boneState->nextState.positionTick = tick;
        boneState->nextState.rotation.x = ((short)*attrInput++) / 32767.0f;
        boneState->nextState.rotation.y = ((short)*attrInput++) / 32767.0f;
        boneState->nextState.rotation.z = ((short)*attrInput++) / 32767.0f;
        float wSqrd = 1.0f - boneState->nextState.rotation.x * boneState->nextState.rotation.x + boneState->nextState.rotation.y * boneState->nextState.rotation.y + boneState->nextState.rotation.z * boneState->nextState.rotation.z;

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

struct SKAnimationKeyframe* skApplyKeyframe(struct SKAnimator* animator, struct SKAnimationKeyframe* keyframe, int* hasMore) {
    int hasMoreBones = 0;
    struct SKBoneKeyframe* currentBone = &keyframe->bones[0];
    int hasAnotherKeyframe = 0;

    do {
        // hasMoreBones is 0 the first loop then 1 every loop after that
        // so hasAnotherKeyframe will be 0 if this keyframe is empty, or the null termination keyframe
        hasAnotherKeyframe = hasMoreBones;
        currentBone = skApplyBoneKeyframe(animator, currentBone, keyframe->tick, &hasMoreBones);
    } while (hasMoreBones);

    if (!hasAnotherKeyframe) {
        animator->nextSourceTick = keyframe->tick;
    }

    *hasMore = hasAnotherKeyframe;

    return (struct SKAnimationKeyframe*)currentBone;
}


int skApplyChunk(struct SKAnimator* animator, struct SKAnimationChunk* chunk) {
    struct SKAnimationKeyframe* nextFrame = &chunk->keyframes[0];

    int hasMore = 1;

    while (hasMore) {
        nextFrame = skApplyKeyframe(animator, nextFrame, &hasMore);
    }

    return (int)chunk - (int)nextFrame;
}

void skProcess(OSIoMesg* message) {
    int messageIndex = message - gSKAnimationPool.ioMessages;

    struct SKAnimator* animator = gSKAnimationPool.animatorsForMessages[messageIndex];

    if (animator) {
        gSKAnimationPool.animatorsForMessages[messageIndex] = 0;

        struct SKAnimationChunk* nextChunk = (struct SKAnimationChunk*)message->dramAddr;
        animator->nextSourceChunkSize = nextChunk->nextChunkSize;

        int chunkSize = skApplyChunk(animator, nextChunk);
        animator->nextChunkSource += chunkSize;

        gSKAnimationPool.firstUsedMemory += chunkSize;
    }
}

void skInitDataPool(int numMessages, int poolSize) {
    gSKAnimationPool.numMessages = numMessages;
    gSKAnimationPool.nextMessage = 0;
    gSKAnimationPool.mesgBuffer = malloc(sizeof(OSMesg) * numMessages);
    gSKAnimationPool.ioMessages = malloc(sizeof(OSIoMesg) * numMessages);
    gSKAnimationPool.animatorsForMessages = malloc(sizeof(struct SKAnimator*) * numMessages);
    osCreateMesgQueue(&gSKAnimationPool.mesgQueue, gSKAnimationPool.mesgBuffer, numMessages);
    gSKAnimationPool.dataPool = malloc(poolSize);
    gSKAnimationPool.dataPoolSize = poolSize;
    gSKAnimationPool.nextAvailableMemory = gSKAnimationPool.dataPool;
    gSKAnimationPool.firstUsedMemory = gSKAnimationPool.dataPool;

    zeroMemory(gSKAnimationPool.animatorsForMessages, sizeof(struct SKAnimator*) * numMessages);
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

int skGetAvailableMemory() {
    int result = gSKAnimationPool.dataPool + gSKAnimationPool.dataPoolSize - gSKAnimationPool.nextAvailableMemory;

    int precedingMemory = gSKAnimationPool.firstUsedMemory - gSKAnimationPool.dataPool;

    if (precedingMemory > result) {
        return precedingMemory;
    }

    return result;
}

void skRequestChunk(struct SKAnimator* animator) {
    unsigned short chunkSize = animator->nextSourceChunkSize;

    if (chunkSize == 0) {
        return;
    }

    animator->nextSourceChunkSize = 0;

    if (chunkSize > gSKAnimationPool.dataPoolSize || !animator->nextChunkSource) {
        // chunk can't possitbly fit exit early
        return;
    }

    unsigned short retries = 0;

    // wait until enough memory is avaialble
    while (skGetAvailableMemory() < chunkSize ||
        gSKAnimationPool.mesgQueue.validCount == gSKAnimationPool.mesgQueue.msgCount) {
        skelatoolWaitForNextMessage();
        ++retries;

        // something is wrong, avoid an infinite loop
        if (retries == gSKAnimationPool.numMessages) {
            return;
        }
    }

    char* dest = gSKAnimationPool.nextAvailableMemory;
    // determine where to place new chunk
    if (gSKAnimationPool.dataPool + gSKAnimationPool.dataPoolSize - dest < chunkSize) {
        dest = gSKAnimationPool.dataPool;
    }

    // request new chunk
    OSIoMesg* ioMesg = &gSKAnimationPool.ioMessages[gSKAnimationPool.nextMessage];

    ioMesg->dramAddr = (void*)dest;
    ioMesg->devAddr = (u32)animator->nextChunkSource;
    ioMesg->size = chunkSize;
    ioMesg->hdr.pri = OS_MESG_PRI_NORMAL;
    ioMesg->hdr.retQueue = &gSKAnimationPool.mesgQueue;

    osEPiStartDma(gSKAnimationPool.handle, ioMesg, OS_READ);
    gSKAnimationPool.animatorsForMessages[gSKAnimationPool.nextMessage] = animator;

    gSKAnimationPool.nextMessage = (gSKAnimationPool.nextMessage + 1) % gSKAnimationPool.numMessages;
}

void skFixedVector3ToFloat(struct SKU16Vector3* input, struct Vector3* output) {
    output->x = (float)input->x / 256.0f;
    output->y = (float)input->y / 256.0f;
    output->z = (float)input->z / 256.0f;
}

void skApplyBoneAnimation(struct SKBoneAnimationState* animatedBone, struct Transform* output, u16 tick) {
    float positionLerp = ((float)tick - (float)animatedBone->prevState.positionTick) / ((float)animatedBone->nextState.positionTick - (float)animatedBone->prevState.positionTick);
    struct Vector3 srcPos;
    skFixedVector3ToFloat(&animatedBone->prevState.position, &srcPos);
    skFixedVector3ToFloat(&animatedBone->nextState.position, &output->position);
    vector3Lerp(&srcPos, &output->position, positionLerp, &output->position);

    float rotationLerp = ((float)tick - (float)animatedBone->prevState.rotationTick) / ((float)animatedBone->nextState.rotationTick - (float)animatedBone->prevState.rotationTick);
    quatLerp(&animatedBone->prevState.rotation, &animatedBone->nextState.rotation, rotationLerp, &output->rotation);

    float scaleLerp = ((float)tick - (float)animatedBone->prevState.scaleTick) / ((float)animatedBone->nextState.scaleTick - (float)animatedBone->prevState.scaleTick);
    skFixedVector3ToFloat(&animatedBone->prevState.scale, &srcPos);
    skFixedVector3ToFloat(&animatedBone->nextState.scale, &output->scale);
    vector3Lerp(&srcPos, &output->scale, scaleLerp, &output->scale);
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

void skAnimatorUpdate(struct SKAnimator* animator, struct SKObject* object) {
    if (!(animator->flags & SKAnimatorFlagsActive) || !animator->currentAnimation) {
        return;
    }
    
    animator->currTick = animator->nextTick;
    animator->currentTime += gTimeDelta;
    animator->nextTick = (u16)(animator->currentTime * animator->currentAnimation->ticksPerSecond);

    if (animator->currTick <= animator->currentAnimation->maxTicks) {
        for (unsigned i = 0; i < object->numberOfBones && i < animator->boneCount; ++i) {
            skApplyBoneAnimation(&animator->boneState[i], &object->boneTransforms[i], animator->currTick);
        }
    }

    if (animator->flags & SKAnimatorFlagsLoop) {
        if (animator->nextTick == animator->currentAnimation->maxTicks) {
            skAnimatorRunClip(animator, animator->currentAnimation, animator->flags);
        }
    }
    
    // queue up next keyframes if they are needed
    if (animator->nextTick >= animator->nextSourceTick) {
        animator->nextSourceTick = TICK_UNDEFINED;
        skRequestChunk(animator);
    }
}