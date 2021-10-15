#ifndef _ROM_UTIL_H
#define _ROM_UTIL_H

#include "memory.h"

void romCopy(const char *src, const char *dest, const int len);

#define LOAD_SEGMENT(segmentName, dest)                                 \
    len = (u32)(_ ## segmentName ## SegmentRomEnd - _ ## segmentName ## SegmentRomStart); \
    dest = malloc(len);                                     \
    romCopy(_ ## segmentName ## SegmentRomStart, dest, len);

#endif