#ifndef _GFX_VALIDATOR_ERROR_PRINTER_H
#define _GFX_VALIDATOR_ERROR_PRINTER_H

#include "validator.h"

typedef void (*gfxPrinter)(void* data, char* output, unsigned outputLength);

struct GFXStringPrinter {
    char* output;
    unsigned int currentIndex;
    unsigned int outputLength;
};

void gfxInitStringPrinter(struct GFXStringPrinter* printer, char* output, unsigned outputLength);
void gfxStringPrinter(void* data, char* output, unsigned outputLength);
void gfxGenerateReadableMessage(struct GFXValidationResult* result, gfxPrinter printer, void* printerData);

#endif