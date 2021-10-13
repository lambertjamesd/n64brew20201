
#define MATRIX_TRANSFORM_SEGMENT                0xC
#define MATRIX_TRANSFORM_SEGMENT_ADDRESS        (MATRIX_TRANSFORM_SEGMENT << 24)

#define CHARACTER_ANIMATION_SEGMENT             0x04000000

#define ANIM_DATA_ROM_ADDRESS(segmentStart, segmentedAddress) ((void*)((u32)(segmentedAddress) - CHARACTER_ANIMATION_SEGMENT + (u32)segmentStart))