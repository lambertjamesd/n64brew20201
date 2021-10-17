#ifndef _SKELATOOL_ANIMATION_CLIP_H
#define _SKELATOOL_ANIMATION_CLIP_H


enum SKBoneAttrMask {
    SKBoneAttrMaskPosition = (1 << 0),
    SKBoneAttrMaskRotation = (1 << 1),
    SKBoneAttrMaskScale = (1 << 2),
};

struct SKBoneKeyframe {
    unsigned char boneIndex;
    unsigned char usedAttributes;
    // each bit set in usedAttributes has 3 entries here
    short attributeData[];
};

struct SKAnimationKeyframe {
    unsigned short tick;
    unsigned short boneCount;
    struct SKBoneKeyframe bones[];
};

struct SKAnimationChunk {
    unsigned short nextChunkSize;
    unsigned short nextChunkTick;
    unsigned short keyframeCount;
    struct SKAnimationKeyframe keyframes[];
};

struct SKAnimationHeader {
    unsigned short firstChunkSize;
    unsigned short ticksPerSecond;
    unsigned short maxTicks;
    struct SKAnimationChunk* firstChunk;
};

#endif