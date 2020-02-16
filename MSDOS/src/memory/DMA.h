#ifndef DMA_H
#define DMA_H

#ifdef DMA_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef struct DMAInfo {
    unsigned int page;
    unsigned char addressLow;
    unsigned char addressHigh;
    unsigned char lengthLow;
    unsigned char lengthHigh;
} DMAInfo;

DMAInfo DMA_makeDMAInfo(unsigned char *buffer, unsigned long length);

#undef DMA_IMPORT
#undef EXTERN

#endif