#ifndef RADPLAY_H
#define RADPLAY_H

#include "Error.h"

#ifdef RADPLAY_IMPORT
#define EXTERN
#else
#define EXTERN extern
#endif

#ifndef RAD_DETECT_REPEATS
#define RAD_DETECT_REPEATS  1
#endif

EXTERN ErrorState RADPlayer_Init(const void *tune, void (*opl3)(void *, unsigned int, unsigned char), void *arg);
EXTERN void RADPlayer_Stop();
EXTERN unsigned char RADPlayer_Update();
EXTERN int RADPlayer_GetHertz();
EXTERN int RADPlayer_GetPlayTimeInSeconds();
EXTERN int RADPlayer_GetTunePos();
EXTERN int RADPlayer_GetTuneLength();
EXTERN int RADPlayer_GetTuneLine();
EXTERN void RADPlayer_SetMasterVolume(int vol);
EXTERN int RADPlayer_GetMasterVolume();
EXTERN int RADPlayer_GetSpeed();

#if RAD_DETECT_REPEATS
EXTERN unsigned long RADPlayer_ComputeTotalTime();
#endif

#undef RADPLAY_IMPORT
#undef EXTERN

#endif