#ifndef _SKELATOOL_ANIMATION_CLIP_H
#define _SKELATOOL_ANIMATION_CLIP_H


enum SKBoneAttrMask {
    SKBoneAttrMaskPosition = (1 << 0),
    SKBoneAttrMaskRotation = (1 << 1),
    SKBoneAttrMaskScale = (1 << 2),
};

struct SKBoneKeyframe {
    // 3 lsbits used to mask which attributes are set
    // 13 msbits are the bone index
    // if usedAttributes is 0, it null terminates the bones list in 
    // SKAnimationKeyframe
    unsigned short usedAttributes;
    // each bit set in the 3 lsbits of usedAttributes has 3 entries here
    unsigned short attributeData[];
};

struct SKAnimationKeyframe {
    unsigned short tick;
    struct SKBoneKeyframe bones[];
};

struct SKAnimationChunk {
    unsigned short nextChunkSize;
    // the tick in the last keyfrome indicates the 
    // tick of the next chunk
    struct SKAnimationKeyframe keyframes[];
};

struct SKAnimationHeader {
    unsigned short firstChunkSize;
    unsigned short ticksPerSecond;
    unsigned short maxTicks;
    struct SKAnimationChunk* firstChunk;
};

#endif