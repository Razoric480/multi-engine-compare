#include <dos.h>

#include "memory\PMem.h"

#define DMA_IMPORT
#include "memory\DMA.h"

DMAInfo DMA_makeDMAInfo(unsigned char *buffer, unsigned long length) {
    DMAInfo info;
    unsigned int segment = FP_SEG(buffer);
    unsigned int offset = FP_OFF(buffer);
    unsigned long address = segment * 16 + offset;
    unsigned int dmaLength = length-1;

    info.addressLow = LO(address);
    info.addressHigh = HI(address);
    info.lengthLow = LO(dmaLength);
    info.lengthHigh = HI(dmaLength);
    info.page = address / 0x10000;
    
    return info;
}