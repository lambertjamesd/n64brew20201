
#include "validator.h"
#include <string.h>
#include "defs.h"

typedef enum GFXValidatorError (*CommandValidator)(struct GFXValidatorState* state, Gfx* at);

extern CommandValidator gfxCommandValidators[GFX_MAX_COMMAND_LEN];

#define SEGMENT_UNINITIALIZED   -1

void gfxInitState(struct GFXValidatorState* state, struct GFXValidationResult* result) {
    int i;

    for (i = 0; i < GFX_MAX_SEGMENTS; ++i) {
        state->segments[i] = SEGMENT_UNINITIALIZED;
    }

    state->result = result;

    state->matrixStackSize = 0;
    state->result->gfxStackSize = 0;
    state->result->reason = GFXValidatorErrorNone;
    state->flags = 0;
}

enum GFXValidatorError gfxPush(struct GFXValidatorState* state, Gfx* translatedNext, Gfx* next) {
    if (state->result->gfxStackSize == GFX_MAX_GFX_STACK) {
        return GFXValidatorStackOverflow;
    } else {
        state->result->gfxTranslatedStack[(unsigned)state->result->gfxStackSize] = translatedNext;
        state->result->gfxVirtualStack[(unsigned)state->result->gfxStackSize] = next;
        state->result->gfxStackSize++;
        return GFXValidatorErrorNone;
    }
}

void gfxPop(struct GFXValidatorState* state) {
    state->result->gfxStackSize--;
}

int gfxIsAligned(int addr, int to) {
    return (addr & ~(to - 1)) == addr;
}

int gfxIsInRam(int addr) {
    return addr >= 0 && addr < osMemSize;
}

enum GFXValidatorError gfxTranslateAddress(struct GFXValidatorState* state, int address, int* output) {
    int segment = _SHIFTR(address, 24, 4);

    if (segment < 0 || segment >= 16 || state->segments[segment] == SEGMENT_UNINITIALIZED) {
        return GFXValidatorSegmentError;
    } else {
        *output = state->segments[segment] + (address & 0xFFFFFF);
    }

    return GFXValidatorErrorNone;
}

enum GFXValidatorError gfxValidateAddress(struct GFXValidatorState* state, int address, int alignedTo) {
    int translated;
    enum GFXValidatorError result = gfxTranslateAddress(state, address, &translated);

    if (result != GFXValidatorErrorNone) {
        return result;
    }

    if (!gfxIsAligned(translated, alignedTo)) {
        return GFXValidatorDataAlignment;
    }

    if (!gfxIsInRam(translated)) {
        return GFXValidatorInvalidAddress;
    }

    return GFXValidatorErrorNone;
}

enum GFXValidatorError gfxValidateNoop(struct GFXValidatorState* state, Gfx* at) {
    if (at->dma.addr != 0 || at->dma.len != 0 || at->dma.par != 0) {
        return GFXValidatorInvalidArguments;
    } else {
        return GFXValidatorErrorNone;
    }
}

enum GFXValidatorError gfxValidateMtx(struct GFXValidatorState* state, Gfx* at) {
    int flags = DMA_MM_IDX(at);
    int len = DMA_MM_LEN(at);

#ifdef F3DEX_GBI_2
    flags ^= G_MTX_PUSH;
#endif

    if (len != sizeof(Mtx)) {
        state->result->reasonParam0 = 0;
        return GFXValidatorInvalidArguments;
    } else if (flags < 0 || flags > (G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_PUSH)) {
        return GFXValidatorInvalidArguments;
    } else if ((flags & G_MTX_PUSH) && state->matrixStackSize == GFX_MAX_MATRIX_STACK) {
        return GFXValidatorStackOverflow;
    } else if ((flags & G_MTX_PUSH) && (flags & G_MTX_PROJECTION)) {
        return GFXValidatorInvalidArguments;
    } else {
        if (!(flags & G_MTX_LOAD)) {
            if (flags & G_MTX_PROJECTION) {
               if (!(state->flags & GFX_INITIALIZED_PMTX)) {
                   return GFXValidatorUnitialized;
               }
            } else {
               if (!(state->flags & GFX_INITIALIZED_MMTX)) {
                   return GFXValidatorUnitialized;
               }
            }
        }

        if ((flags & G_MTX_PUSH)) {
            ++state->matrixStackSize;
        }

        if (flags & G_MTX_PROJECTION) {
            state->flags |= GFX_INITIALIZED_PMTX;
        } else {
            state->flags |= GFX_INITIALIZED_MMTX;
        }

        return gfxValidateAddress(state, at->dma.addr, 8);
    }
}

enum GFXValidatorError gfxValidateMoveMem(struct GFXValidatorState* state, Gfx* at) {
    // TODO F3DEX_GBI_2
#ifdef F3DEX_GBI_2
    return GFXValidatorErrorNone;
#else

    int location = DMA_MM_IDX(at);
    int expectedLen = 0;

    switch (location) {
        case G_MV_VIEWPORT:
            expectedLen = sizeof(Vp);
            break;
        case G_MV_LOOKATY:
        case G_MV_LOOKATX:
            expectedLen = sizeof(Light);
            break;
        case G_MV_L0:
        case G_MV_L1:
        case G_MV_L2:
        case G_MV_L3:
        case G_MV_L4:
        case G_MV_L5:
        case G_MV_L6:
        case G_MV_L7:
            expectedLen = sizeof(Light);
            break;
        case G_MV_TXTATT:
            // Not sure what to expect here
            expectedLen = DMA_MM_LEN(at);
            break;
        case G_MV_MATRIX_1:
        case G_MV_MATRIX_2:
        case G_MV_MATRIX_3:
        case G_MV_MATRIX_4:
            expectedLen = 16;
            break;
        default:
            return GFXValidatorInvalidArguments;
    }

    if (expectedLen != DMA_MM_LEN(at)) {
        return GFXValidatorInvalidArguments;
    }
    
    return gfxValidateAddress(state, at->dma.addr, 8);
#endif
}

enum GFXValidatorError gfxValidateVertex(struct GFXValidatorState* state, Gfx* at) {
#ifdef F3DEX_GBI_2

#else
    int vtxCount = (DMA1_PARAM(at) >> 4) + 1;

    if (vtxCount * sizeof(Vtx) != DMA1_LEN(at)) {
        return GFXValidatorInvalidArguments;
    }
#endif
    return gfxValidateAddress(state, at->dma.addr, 8);
}


enum GFXValidatorError gfxValidateDL(struct GFXValidatorState* state, Gfx* at) {
    if (DMA1_LEN(at) != 0) {
        return GFXValidatorInvalidArguments;
    } else if (DMA1_PARAM(at) != (DMA1_PARAM(at) & (G_DL_NOPUSH | G_DL_PUSH))) {
        return GFXValidatorInvalidArguments;
    } else {
        return gfxValidateAddress(state, at->dma.addr, 8);
    }
}


enum GFXValidatorError gfxValidateSprite2DBase(struct GFXValidatorState* state, Gfx* at) {
    if (DMA1_LEN(at) != sizeof(uSprite) || DMA1_PARAM(at) != 0) {
        return GFXValidatorInvalidArguments;
    } else {
        return gfxValidateAddress(state, at->dma.addr, 8);
    }
}

enum GFXValidatorError gfxValidateTri1(struct GFXValidatorState* state, Gfx* at) {
    u8 v0 = _SHIFTR(at->words.w1, 16, 8);
    u8 v1 = _SHIFTR(at->words.w1, 8, 8);
    u8 v2 = _SHIFTR(at->words.w1, 0, 8);
    u8 flag = _SHIFTR(at->words.w1, 24, 8);

    if (v0 > 150 || v1 > 150 || v2 > 150 || flag > 3) {
        return GFXValidatorInvalidArguments;
    } else {
        return GFXValidatorErrorNone;
    }
}

enum GFXValidatorError gfxValidateTri2(struct GFXValidatorState* state, Gfx* at) {
    return GFXValidatorErrorNone;
}

enum GFXValidatorError gfxValidateCullDL(struct GFXValidatorState* state, Gfx* at) {
    // TODO handle G_SPRITE2D_SCALEFLIP in sprite mode
    u8 v0 = at->words.w0 & 0xFFFFFF;
    u8 v1 = at->words.w1;

    if ((v1 - 40) >= v0) {
        return GFXValidatorInvalidArguments;
    } else {
        return GFXValidatorErrorNone;
    }
}

enum GFXValidatorError gfxValidatePopMtx(struct GFXValidatorState* state, Gfx* at) {
    // TODO handle G_SPRITE2D_DRAW in sprite mode
    if (state->matrixStackSize == 0) {
        return GFXValidatorStackUnderflow;
    } else if (at->words.w1 != G_MTX_MODELVIEW) {
        return GFXValidatorInvalidArguments;
    } else {
        --state->matrixStackSize;
        return GFXValidatorErrorNone;
    }
}


enum GFXValidatorError gfxValidateMoveWord(struct GFXValidatorState* state, Gfx* at) {
    int index = MOVE_WORD_IDX(at);
    int offset = MOVE_WORD_OFS(at);
    int data = MOVE_WORD_DATA(at);

    switch (index) {
        case G_MW_SEGMENT:
            if (offset > GFX_MAX_SEGMENTS * 4 || offset < 0) {
                return GFXValidatorInvalidArguments;
            } else if (data < 0 || data > osMemSize) {
                return GFXValidatorInvalidArguments;
            }

            state->segments[offset/4] = data;
            break;
        case G_MW_CLIP:
            break;
        case G_MW_MATRIX:
            break;
#ifdef F3DEX_GBI_2
        case G_MW_FORCEMTX:
            break;
#else
        case G_MW_POINTS:
            break;
#endif
        case G_MW_NUMLIGHT:
            break;
        case G_MW_LIGHTCOL:
            break;
        case G_MW_FOG:
            break;
        case G_MW_PERSPNORM:
            break;
        default:
            return GFXValidatorInvalidArguments;
    }

    return GFXValidatorErrorNone;
}

enum GFXValidatorError gfxValidateSetCImg(struct GFXValidatorState* state, Gfx* at) {
    int format = _SHIFTR(at->words.w0, 21, 3);
    int size = _SHIFTR(at->words.w0, 19, 2);
    // int width = _SHIFTR(at->words.w0, 0, 12) + 1;

    if (format == G_IM_FMT_RGBA) {
        if (size != G_IM_SIZ_16b && size != G_IM_SIZ_32b) {
            return GFXValidatorInvalidArguments;
        }
    } else if (format == G_IM_FMT_CI) {
        if (size != G_IM_SIZ_8b) {
            return GFXValidatorInvalidArguments;
        }
    } else if (format == G_IM_FMT_IA) {
        if (size != G_IM_SIZ_8b && size != G_IM_SIZ_16b) {
            return GFXValidatorInvalidArguments;
        }
    } else if (format == G_IM_FMT_I) {
        if (size != G_IM_SIZ_4b && size != G_IM_SIZ_8b) {
            return GFXValidatorInvalidArguments;
        }
    } else {
        return GFXValidatorInvalidArguments;
    }

    return gfxValidateAddress(state, at->words.w1, 64);
}

enum GFXValidatorError gfxValidateTODO(struct GFXValidatorState* state, Gfx* at) {
    return GFXValidatorErrorNone;
}

enum GFXValidatorError gfxValidateList(struct GFXValidatorState* state, Gfx* gfx, Gfx* segmentGfx) {
    enum GFXValidatorError result = gfxPush(state, gfx, segmentGfx);

    if (result != GFXValidatorErrorNone) {
        return result;
    }

    Gfx** stackVirtualTarget = &state->result->gfxVirtualStack[state->result->gfxStackSize - 1];
    Gfx** stackTranslatedTarget = &state->result->gfxTranslatedStack[state->result->gfxStackSize - 1];

    int active = 1;

    while (active) {
        int commandType = _SHIFTR(gfx->words.w0, 24, 8);

        if (commandType < 0 || commandType >= GFX_MAX_COMMAND_LEN) {
            return GFXValidatorInvalidCommand;
        }

        CommandValidator validator = gfxCommandValidators[commandType];

        if (!validator) {
            return GFXValidatorInvalidCommand;
        }

        result = validator(state, gfx);

        if (result != GFXValidatorErrorNone) {
            return result;
        }

        switch (commandType) {
            case (u8)G_ENDDL:
                active = 0;
                break;
            case (u8)G_DL:
                {
                    int next;
                    result = gfxTranslateAddress(state, gfx->dma.addr, &next);

                    if (result != GFXValidatorErrorNone) {
                        return result;
                    }

                    next = PHYS_TO_K0(next);

                    if (gfx->dma.par == G_DL_NOPUSH) {
                        gfx = (Gfx*)next;
                        *stackVirtualTarget = (Gfx*)gfx->dma.addr;
                        *stackTranslatedTarget = (Gfx*)next;
                    } else {
                        result = gfxValidateList(state, (Gfx*)next, (Gfx*)gfx->dma.addr);

                        if (result != GFXValidatorErrorNone) {
                            return result;
                        }

                        ++gfx;
                        ++(*stackVirtualTarget);
                        ++(*stackTranslatedTarget);
                    }
                }
                break;
            default:
                ++gfx;
                ++(*stackVirtualTarget);
                ++(*stackTranslatedTarget);
                break;
        };
    }

    gfxPop(state);
    return GFXValidatorErrorNone;
}

enum GFXValidatorError gfxValidate(OSTask* task, int maxGfxCount, struct GFXValidationResult* validateResult) {
    struct GFXValidatorState state;
    gfxInitState(&state, validateResult);
    
    if (task->t.type == M_GFXTASK) {
        enum GFXValidatorError result = gfxValidateList(&state, (Gfx*)task->t.data_ptr, (Gfx*)task->t.data_ptr);
        
        if (result != GFXValidatorErrorNone) {
            validateResult->reason = result;
            return result;
        }
    }

    return GFXValidatorErrorNone;
}

CommandValidator gfxCommandValidators[GFX_MAX_COMMAND_LEN] = {
    [G_SPNOOP] = gfxValidateNoop,
    [G_MTX] = gfxValidateMtx,
    [G_MOVEMEM] = gfxValidateMoveMem,
    [G_VTX] = gfxValidateVertex,
    [G_DL] = gfxValidateDL,
#ifdef G_SPRITE2D_BASE
    [G_SPRITE2D_BASE] = gfxValidateSprite2DBase,
#endif

    [(u8)G_TRI1] = gfxValidateTri1,
#ifdef G_TRI2
    [(u8)G_TRI2] = gfxValidateTri2,
#endif
    [(u8)G_CULLDL] = gfxValidateCullDL,
    [(u8)G_POPMTX] = gfxValidatePopMtx,
    [(u8)G_MOVEWORD] = gfxValidateMoveWord,

#ifdef G_MODIFYVTX
    [(u8)G_MODIFYVTX] = gfxValidateTODO,
#endif
#ifdef G_BRANCH_Z
    [(u8)G_BRANCH_Z] = gfxValidateTODO,
#endif
#ifdef G_QUAD
    [(u8)G_QUAD] = gfxValidateTODO,
#endif
#ifdef G_SPECIAL_1
    [(u8)G_SPECIAL_1] = gfxValidateTODO,
#endif
#ifdef G_SPECIAL_2
    [(u8)G_SPECIAL_2] = gfxValidateTODO,
#endif
#ifdef G_SPECIAL_3
    [(u8)G_SPECIAL_3] = gfxValidateTODO,
#endif
#ifdef G_DMA_IO
    [(u8)G_DMA_IO] = gfxValidateTODO,
#endif
    [(u8)G_TEXTURE] = gfxValidateTODO,
    [(u8)G_SETOTHERMODE_H] = gfxValidateTODO,
    [(u8)G_SETOTHERMODE_L] = gfxValidateTODO,
    [(u8)G_ENDDL] = gfxValidateTODO,
#ifdef G_GEOMETRYMODE
    [(u8)G_GEOMETRYMODE] = gfxValidateTODO,
#endif
#ifdef G_SETGEOMETRYMODE
    [(u8)G_SETGEOMETRYMODE] = gfxValidateTODO,
#endif
#ifdef G_CLEARGEOMETRYMODE
    [(u8)G_CLEARGEOMETRYMODE] = gfxValidateTODO,
#endif
    [(u8)G_LINE3D] = gfxValidateTODO,
    [(u8)G_RDPHALF_1] = gfxValidateTODO,
    [(u8)G_RDPHALF_2] = gfxValidateTODO,
#ifdef G_RDPHALF_CONT
    [(u8)G_RDPHALF_CONT] = gfxValidateTODO,
#endif
#ifdef G_LOAD_UCODE
    [(u8)G_LOAD_UCODE] = gfxValidateMoveWord,
#endif

    [(u8)G_NOOP] = gfxValidateTODO,

    [(u8)G_SETCIMG] = gfxValidateSetCImg,
    [(u8)G_SETZIMG] = gfxValidateTODO,
    [(u8)G_SETTIMG] = gfxValidateTODO,
    [(u8)G_SETCOMBINE] = gfxValidateTODO,
    [(u8)G_SETENVCOLOR] = gfxValidateTODO,
    [(u8)G_SETPRIMCOLOR] = gfxValidateTODO,
    [(u8)G_SETBLENDCOLOR] = gfxValidateTODO,
    [(u8)G_SETFOGCOLOR] = gfxValidateTODO,
    [(u8)G_SETFILLCOLOR] = gfxValidateTODO,
    [(u8)G_FILLRECT] = gfxValidateTODO,
    [(u8)G_SETTILE] = gfxValidateTODO,
    [(u8)G_LOADTILE] = gfxValidateTODO,
    [(u8)G_LOADBLOCK] = gfxValidateTODO,
    [(u8)G_SETTILESIZE] = gfxValidateTODO,
    [(u8)G_LOADTLUT] = gfxValidateTODO,
    [(u8)G_RDPSETOTHERMODE] = gfxValidateTODO,
    [(u8)G_SETPRIMDEPTH] = gfxValidateTODO,
    [(u8)G_SETSCISSOR] = gfxValidateTODO,
    [(u8)G_SETCONVERT] = gfxValidateTODO,
    [(u8)G_SETKEYR] = gfxValidateTODO,
    [(u8)G_SETKEYGB] = gfxValidateTODO,
    [(u8)G_RDPFULLSYNC] = gfxValidateTODO,
    [(u8)G_RDPTILESYNC] = gfxValidateTODO,
    [(u8)G_RDPPIPESYNC] = gfxValidateTODO,
    [(u8)G_RDPLOADSYNC] = gfxValidateTODO,
    [(u8)G_TEXRECTFLIP] = gfxValidateTODO,
    [(u8)G_TEXRECT] = gfxValidateTODO,
};