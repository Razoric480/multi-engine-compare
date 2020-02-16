#include <dos.h>

#include "Timer.h"

#define JOY_IMPORT
#include "input\Joy.h"

#define JOYSTICK_PORT   0x201

#define JOY_A_BUTTON_1  0x10
#define JOY_A_BUTTON_2  0x20
#define JOY_B_BUTTON_1  0x40
#define JOY_B_BUTTON_2  0x80
#define JOY_A_X         1
#define JOY_A_Y         2
#define JOY_B_X         4
#define JOY_B_Y         8

enum JoyButtonStatus {
    JOY_PRESSED = 1,
    JOY_HELD = 2,
    JOY_RELEASED = -1,
    JOY_INACTIVE = 0
};

static unsigned int stickPositions[4];
static int buttons[4];
static unsigned long next, rate;

static void joystickPositions(unsigned int stickValues[4]) {
    unsigned int i;
    unsigned char done[4];
    stickValues[0] = stickValues[1] = stickValues[2] = stickValues[3];
    done[0] = done[1] = done[2] = done[3] = 0;
    outp(JOYSTICK_PORT, 1);
    for(i=0; i<0xFFFF; ++i) {
        unsigned char status = inp(JOYSTICK_PORT);
        if(!done[0]) {
            if((status & JOY_A_X) == 0) {
                done[0] = 1;
            }
            else {
                stickValues[0]++;
            }
        }
        if(!done[1]) {
            if((status & JOY_A_Y) == 0) {
                done[1] = 1;
            }
            else {
                stickValues[1]++;
            }
        }
        if(!done[2]) {
            if((status & JOY_B_X) == 0) {
                done[2] = 1;
            }
            else {
                stickValues[2]++;
            }
        }
        if(!done[3]) {
            if((status & JOY_B_Y) == 0) {
                done[3] = 1;
            }
            else {
                stickValues[3]++;
            }
        }

        if(done[0] && done[1] && done[2] && done[3]) {
            break;
        }
    }
}

static unsigned char joystickButton(unsigned char button) {
    return (inp(JOYSTICK_PORT) & button) == 0;
}

static void updateButtons(void) {
    unsigned char status = inp(JOYSTICK_PORT);

    JoyButton currentButton = JOY_A_1;
    unsigned char buttonStatus = ((status & currentButton) == 0);
    if(buttonStatus) {
        if(buttons[currentButton] < 1) {
            buttons[currentButton] = JOY_PRESSED;
        }
    }
    else {
        if(buttons[currentButton] > 0) {
            buttons[currentButton] = JOY_RELEASED;
        }
    }

    currentButton = JOY_A_2;
    buttonStatus = ((status & currentButton) == 0);
    if(buttonStatus) {
        if(buttons[currentButton] < 1) {
            buttons[currentButton] = JOY_PRESSED;
        }
    }
    else {
        if(buttons[currentButton] > 0) {
            buttons[currentButton] = JOY_RELEASED;
        }
    }

    currentButton = JOY_B_1;
    buttonStatus = ((status & currentButton) == 0);
    if(buttonStatus) {
        if(buttons[currentButton] < 1) {
            buttons[currentButton] = JOY_PRESSED;
        }
    }
    else {
        if(buttons[currentButton] > 0) {
            buttons[currentButton] = JOY_RELEASED;
        }
    }

    currentButton = JOY_B_2;
    buttonStatus = ((status & currentButton) == 0);
    if(buttonStatus) {
        if(buttons[currentButton] < 1) {
            buttons[currentButton] = JOY_PRESSED;
        }
    }
    else {
        if(buttons[currentButton] > 0) {
            buttons[currentButton] = JOY_RELEASED;
        }
    }
}

ErrorState Joy_initialize(void) {
    unsigned char i;
    rate = 1;
    next = 1;
    updateButtons();
    joystickPositions(stickPositions);
    if(stickPositions[0] == 0xFFFF) {
        return ERR_HARDWARE_NOT_FOUND;
    }

    return ERR_SUCCESS;
}

void Joy_update(void) {
    if(Timer_getFastTick() > next) {
        unsigned char i;
        for(i=0; i<4; ++i) {
            if(buttons[i] == JOY_PRESSED) {
                buttons[i] = JOY_HELD;
            }
            else if(buttons[i] == JOY_RELEASED) {
                buttons[i] = JOY_INACTIVE;
            }
        }

        disable();
        updateButtons();
        joystickPositions(stickPositions);
        enable();

        next += rate;
    }
}

unsigned int Joy_Axis(JoyAxis axis) {
    return stickPositions[axis];
}

unsigned char Joy_buttonPressed(JoyButton button) {
    return buttons[button] == JOY_PRESSED;
}

unsigned char Joy_buttonDown(JoyButton button) {
    return buttons[button] > 0;
}

unsigned char Joy_buttonReleased(JoyButton button) {
    return buttons[button] == JOY_RELEASED;
}

unsigned char Joy_buttonUp(JoyButton button) {
    return buttons[button] < 1;
}