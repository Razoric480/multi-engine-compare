#ifndef FMSYNTH_H
#define FMSYNTH_H

#ifdef FMSYNTH_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef void (*OPLWrite)(void *, unsigned int, unsigned char);

EXTERN void FMSynth_init(unsigned int baseAddress);
EXTERN void FMSynth_terminate(void);
EXTERN OPLWrite FMSynth_getOPLControls(void);
EXTERN unsigned char FMSynth_initialized(void);

#undef FMSYNTH_IMPORT
#undef EXTERN

#endif