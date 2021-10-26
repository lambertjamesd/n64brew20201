
#ifndef _GFX_VALIDATOR_VALIDATOR_H
#define _GFX_VALIDATOR_VALIDATOR_H

#include <ultra64.h>

#define GFX_MAX_COMMAND_LEN     256
#define GFX_MAX_SEGMENTS        16
#define GFX_MAX_GFX_STACK       10
#define GFX_MAX_MATRIX_STACK    10

#define GFX_INITIALIZED_PMTX    (1 << 0)
#define GFX_INITIALIZED_MMTX    (1 << 1)

enum GFXValidatorError {
    GFXValidatorErrorNone,
    GFXValidatorStackOverflow,
    GFXValidatorStackUnderflow,
    GFXValidatorInvalidCommand,
    GFXValidatorSegmentError,
    GFXValidatorDataAlignment,
    GFXValidatorInvalidAddress,
    GFXValidatorInvalidArguments,
    GFXValidatorUnitialized,
    GFXValidatorErrorCount,
};

struct GFXValidationResult {
    Gfx* gfxVirtualStack[GFX_MAX_GFX_STACK];
    Gfx* gfxTranslatedStack[GFX_MAX_GFX_STACK];
    char gfxStackSize;
    enum GFXValidatorError reason;
    unsigned reasonParam0;
};

struct GFXValidatorState {
    struct GFXValidationResult* result;
    int segments[GFX_MAX_SEGMENTS];
    short matrixStackSize;
    int flags;
};

enum GFXValidatorError gfxValidate(OSTask* task, int maxGfxCount, struct GFXValidationResult* result);

#endif