
#include "gfxvalidator/command_printer.h"
#include "gfxvalidator/error_printer.h"
#include <string.h>
#include <stdio.h>

#define TMP_BUFFER_SIZE 64

typedef unsigned (*ErrorPrinter)(struct GFXValidationResult* result, char* output, unsigned maxOutputLen);

unsigned gfxUnknownError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "unknown error %d", result->reason);
}

unsigned gfxStackOverflowError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Stack Overflow");
}

unsigned gfxStackUnderflowError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Stack Underflow");
}

unsigned gfxInvalidCommandError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Invalid Command");
}

unsigned gfxSegmentError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Invalid Segment");
}

unsigned gfxDataAlignmentError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Data Alignment");
}

unsigned gfxInvalidAddressError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Invalid Address");
}

unsigned gfxInvalidArgumentsError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Invalid Arguments");
}

unsigned gfxUnitializedError(struct GFXValidationResult* result, char* output, unsigned maxOutputLen) {
    return sprintf(output, "Unitialized");
}

ErrorPrinter gfxErrorPrinters[GFXValidatorErrorCount] = {
    [(u8)GFXValidatorStackOverflow] = gfxStackOverflowError,
    [(u8)GFXValidatorStackUnderflow] = gfxStackUnderflowError,
    [(u8)GFXValidatorInvalidCommand] = gfxInvalidCommandError,
    [(u8)GFXValidatorSegmentError] = gfxSegmentError,
    [(u8)GFXValidatorDataAlignment] = gfxDataAlignmentError,
    [(u8)GFXValidatorInvalidAddress] = gfxInvalidAddressError,
    [(u8)GFXValidatorInvalidArguments] = gfxInvalidArgumentsError,
    [(u8)GFXValidatorUnitialized] = gfxUnitializedError,
};

void gfxInitStringPrinter(struct GFXStringPrinter* printer, char* output, unsigned outputLength) {
    printer->currentIndex = 0;
    printer->output = output;
    printer->outputLength = outputLength;
}

void gfxStringPrinter(void* data, char* output, unsigned outputLength) {
    struct GFXStringPrinter* printer = (struct GFXStringPrinter*)data;
    unsigned int remaining = printer->outputLength - printer->currentIndex;

    if (outputLength > remaining) {
        outputLength = remaining;
    }

    if (outputLength) {
        strncpy(&printer->output[printer->currentIndex], output, outputLength);
        printer->currentIndex += outputLength;
    }
}

void gfxGenerateReadableMessage(struct GFXValidationResult* result, gfxPrinter printer, void* printerData) {
    char tmpBuffer[TMP_BUFFER_SIZE];

    if (result->reason == GFXValidatorErrorNone) {
        printer(printerData, tmpBuffer, sprintf(tmpBuffer, "success"));
        return;
    }

    for (int i = 0; i < result->gfxStackSize; ++i) {
        char* curr = tmpBuffer;
        unsigned currOffset = 0;
        currOffset += sprintf(curr + currOffset, "0x%08x: ", (unsigned)result->gfxVirtualStack[i]);
        currOffset += gfxPrintCommand(result->gfxTranslatedStack[i], curr + currOffset, (unsigned)(TMP_BUFFER_SIZE - currOffset));

        if (currOffset < TMP_BUFFER_SIZE) {
            curr[currOffset++] = '\n';
        }

        printer(printerData, tmpBuffer, currOffset);
    }

    ErrorPrinter errorPrinter = 0;

    if (result->reason < GFXValidatorErrorCount) {
        errorPrinter = gfxErrorPrinters[result->reason];
    }

    if (!errorPrinter) {
        errorPrinter = gfxUnknownError;
    }

    printer(printerData, tmpBuffer, errorPrinter(result, tmpBuffer, TMP_BUFFER_SIZE));
}