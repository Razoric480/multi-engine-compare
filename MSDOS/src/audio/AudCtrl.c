#include <stdio.h>

#include "audio\FMSynth.h"
#include "audio\RADPlay.h"
#include "audio\SBlaster.h"
#include "Timer.h"

#define AUDCTRL_IMPORT
#include "audio\AudCtrl.h"

static double next, rate;
static unsigned char radPlaying;
static unsigned char initialized;

ErrorState AudioController_init(void) {
    ErrorState error = SBlaster_initialize();
    if(error != ERR_SUCCESS) {
        initialized = 0;
        return error;
    }

    FMSynth_init(SBlaster_baseAddress());
    initialized = 1;

    return ERR_SUCCESS;
}

void AudioController_playRAD(RADFile file) {
    if(!initialized) {
        return;
    }

    if(FMSynth_initialized()) {
        RADPlayer_Init(file, FMSynth_getOPLControls(), 0);
        rate = (double)Timer_getUpdateRate() / (double)RADPlayer_GetHertz();
        next = (double)Timer_getFastTick() + rate;
        radPlaying = 1;
    }
}

void AudioController_stopRAD(void) {
    if(!initialized || !FMSynth_initialized()) {
        return;
    }

    RADPlayer_Stop();
    radPlaying = 0;
}

void AudioController_playWAV(WAVFile file) {
    if(!initialized) {
        return;
    }

    SBlaster_playWAV(file);
}

void AudioController_update(void) {
    if(!initialized) {
        return;
    }

    if(radPlaying && FMSynth_initialized()) {
        if((double)Timer_getFastTick() > next) {
            next += rate;
            RADPlayer_Update();
        }
    }
}

void AudioController_terminate(void) {
    if(!initialized) {
        return;
    }

    if(FMSynth_initialized()) {
        RADPlayer_Stop();
        FMSynth_terminate();
    }
    SBlaster_terminate();
}

unsigned char AudioController_isPlayingWav(void) {
    return SBlaster_isPlaying();
}