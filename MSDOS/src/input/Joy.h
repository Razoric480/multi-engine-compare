#ifndef JOY_H
#define JOY_H

#include "Error.h"

#ifdef JOY_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

typedef enum JoyButton_t {
    JOY_A_1,
    JOY_A_2,
    JOY_B_1,
    JOY_B_2
} JoyButton;

typedef enum JoyAxis_t {
    JOY_A_X,
    JOY_A_Y,
    JOY_B_X,
    JOY_B_Y
} JoyAxis;

EXTERN ErrorState Joy_initialize(void);
EXTERN void Joy_update(void);
EXTERN unsigned int Joy_Axis(JoyAxis axis);
EXTERN unsigned char Joy_buttonPressed(JoyButton button);
EXTERN unsigned char Joy_buttonDown(JoyButton button);
EXTERN unsigned char Joy_buttonReleased(JoyButton button);
EXTERN unsigned char Joy_buttonUp(JoyButton button);

#undef JOY_IMPORT
#undef EXTERN

#endif