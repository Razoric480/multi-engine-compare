#ifndef AUDCTRL_H
#define AUDCTRL_H

#include "audio\RADFile.h"
#include "audio\WAVFile.h"

#ifdef AUDCTRL_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN ErrorState AudioController_init(void);
EXTERN void AudioController_terminate(void);
EXTERN void AudioController_update(void);
EXTERN void AudioController_playRAD(RADFile file);
EXTERN void AudioController_stopRAD(void);
EXTERN void AudioController_playWAV(WAVFile file);
EXTERN unsigned char AudioController_isPlayingWav(void);

#undef AUDCTRL_IMPORT
#undef EXTERN

#endif