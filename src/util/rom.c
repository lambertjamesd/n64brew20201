
#include <ultra64.h>
#include "rom.h"

extern OSMesgQueue dmaMessageQ;
extern OSPiHandle* handler;

void romCopy(const char *src, const char *dest, const int len)
{
    OSIoMesg dmaIoMesgBuf;
    OSMesg dummyMesg;
    
    osInvalDCache((void *)dest, (s32) len);

    dmaIoMesgBuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    dmaIoMesgBuf.hdr.retQueue = &dmaMessageQ;
    dmaIoMesgBuf.dramAddr     = (void*)dest;
    dmaIoMesgBuf.devAddr      = (u32)src;
    dmaIoMesgBuf.size         = (u32)len;

    osEPiStartDma(handler, &dmaIoMesgBuf, OS_READ);
    (void) osRecvMesg(&dmaMessageQ, &dummyMesg, OS_MESG_BLOCK);
}