#include <DOS.h>

#define FMSYNTH_IMPORT
#include "audio\FMSynth.h"

typedef enum {
    NONE, OPL2, DUALOPL2, OPL3,
} OPLStatus;

#define BACK_ADDRESS        0x388
#define BACK_DATA           0x389
#define PRIMARY_ADDRESS     0x000
#define PRIMARY_DATA        0x001

#define OPL3                0x00
#define OPL2                0x06
#define NONE                0xFF
#define REGISTER_THRESHOLD  0x100

OPLStatus status = NONE;
unsigned int primAd;
unsigned int backAd;

extern void OPLWriteImpl(void *argument, unsigned int registerNumber, unsigned char registerValue);

/*static void OPLWriteImpl(void *argument, unsigned int registerNumber, unsigned char registerValue) {
    (void)argument; /* Unused variable */
    /*if(status == OPL3 || status == DUALOPL2) {
        if(registerNumber <= REGISTER_THRESHOLD) {
            outportb(primAd, registerNumber);
            outportb(primAd+1, registerValue);
            if(status == DUALOPL2) {
                outportb(primAd+2, registerNumber);
                outportb(primAd+3, registerValue);
            }
        }
        else {
            outportb(primAd+2, registerNumber);
            outportb(primAd+3, registerValue);
        }
    }
    else {
        outportb(backAd, registerNumber);
        outportb(backAd+1, registerValue);
    }
}*/

void FMSynth_init(unsigned int baseAddress) {
    unsigned char regBack, regPrim, regSec;
    primAd = PRIMARY_ADDRESS + baseAddress;
    backAd = BACK_ADDRESS;

    regBack = inportb(backAd);
    regPrim = inportb(primAd);
    regSec = inportb(primAd+2);

    if(regBack == OPL3) {
        status = OPL3;
    }
    else if(regBack == OPL2 && regPrim == OPL2 && regSec == OPL2) {
        status = DUALOPL2;
    }
    else if(regBack == OPL2 && regPrim == NONE && regSec == NONE) {
        status = OPL2;
    }
    else {
        status = NONE;
    }
}

OPLWrite FMSynth_getOPLControls(void) {
    return &OPLWriteImpl;
}

unsigned char FMSynth_initialized(void) {
    return status != NONE;
}

void FMSynth_terminate(void) {
    status = NONE;
    primAd = backAd = 0;
}