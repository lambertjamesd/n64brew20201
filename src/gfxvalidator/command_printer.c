
#include <string.h>
#include "command_printer.h"
#include "validator.h"
#include "defs.h"

typedef int (*GFXValidatorPrinter)(Gfx* command, char* output, unsigned maxOutputLength);

int gfxUnknownCommandPrinter(Gfx* command, char* output, unsigned maxOutputLength) {
    return sprintf(output, "unknown 0x%08x%08x", (unsigned)command->words.w0, (unsigned)command->words.w1);
}

int gfxDLCommandPrinter(Gfx* command, char* output, unsigned maxOutputLength) {
    if (DMA1_PARAM(command) & G_DL_NOPUSH) {
        return sprintf(output, "gsSPBranchList(0x%08x)", command->words.w1);
    } else {
        return sprintf(output, "gsSPDisplayList(0x%08x)", command->words.w1);
    }
}

int gfxMoveWordCommandPrinter(Gfx* command, char* output, unsigned maxOutputLength) {
    int index = MOVE_WORD_IDX(command);
    int offset = MOVE_WORD_OFS(command);
    int data = MOVE_WORD_DATA(command);

    switch (index) {
        case G_MW_SEGMENT:
            return sprintf(output, "gsSPSegment(0x%x,0x%08x)", offset, data);
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
    }

    return sprintf(output, "gsSPMoveWord(0x%x, 0x%x,0x%08x)", index, offset, data); 
}


int gfxMtxCommandPrinter(Gfx* command, char* output, unsigned maxOutputLength) {
    int flags = DMA_MM_IDX(command);
    int addr = command->dma.addr;

    return sprintf(output, "gsSPMatrix(0x%08x, %x)", addr, flags); 
}

int gfxCImgCommandPrinter(Gfx* command, char* output, unsigned maxOutputLength) {
    int format = _SHIFTR(command->words.w0, 21, 3);
    int size = _SHIFTR(command->words.w0, 19, 2);
    int width = _SHIFTR(command->words.w0, 0, 12) + 1;

    return sprintf(output, "gsDPSetColorImage(%d, %d, %d, 0x%08x)", format, size, width, command->words.w1);
}

GFXValidatorPrinter gfxCommandPrinters[GFX_MAX_COMMAND_LEN] = {
    [(u8)G_MOVEWORD] = gfxMoveWordCommandPrinter,
    [(u8)G_MTX] = gfxMtxCommandPrinter,
    [(u8)G_DL] = gfxDLCommandPrinter,
    [(u8)G_SETCIMG] = gfxCImgCommandPrinter,
};

unsigned gfxPrintCommand(Gfx* command, char* output, unsigned maxOutputLen) {
    int commandType = _SHIFTR(command->words.w0, 24, 8);

    GFXValidatorPrinter printer = gfxCommandPrinters[commandType];

    if (printer) {
        return printer(command, output, maxOutputLen);
    } else {
        return gfxUnknownCommandPrinter(command, output, maxOutputLen);
    }
}