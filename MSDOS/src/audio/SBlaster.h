#ifndef SBLASTER_H
#define SBLASTER_H

#include "audio\WAVFile.h"
#include "Error.h"

#ifdef SBLASTER_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN ErrorState SBlaster_initialize(void);
EXTERN void SBlaster_playWAV(WAVFile file);
EXTERN void SBlaster_terminate(void);
EXTERN unsigned int SBlaster_baseAddress(void);
EXTERN unsigned char SBlaster_isPlaying(void);

#undef SBLASTER_IMPORT
#undef EXTERN

#endif