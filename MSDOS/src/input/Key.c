#include <mem.h>
#include <dos.h>
#include <stdio.h>

#include "Timer.h"

#define KEY_IMPORT
#include "input\Key.h"

#define KEYBOARD_INT    0x09
#define KEY_BUFFER      0x60
#define KEY_CONTROL     0x61
#define INT_CONTROL     0x20

static void interrupt (*oldISR)(void);
static volatile int rawKey;
static volatile char keys[84];

#define IDX(x)          x <= 40 ? x-1 : x <= 83 ? x-2 : x == 87 ? 82 : 83

void interrupt newKeyInt(void) {
    /* disable(); */
    _asm {
        sti; /* enable interrupts */

    /* rawKey = inportb(KEY_BUFFER); */
        in al, KEY_BUFFER; /* get key that was pressed and put it into AL */
        xor ah, ah; /* zero out upper 8 bits of ax */
        mov rawKey, ax; /* store the key in raw_key */

    /* ctrlRegister = inportb(KEY_CONTROL) | 0x82; */
        in al, KEY_CONTROL; /* set control register */
        or al, 82h; /* set proper bits to reset the FF */

    /* outportb(KEY_CONTROL, ctrlRegister); */
        out KEY_CONTROL, al; /* send new data back to control register */

    /* ctrlRegister &= 0x7F; */
        and al, 7fh;

    /* outportb(KEY_CONTROL, ctrlRegister); */
        out KEY_CONTROL, al; /* complete reset */

    /* outportb(INT_CONTROL, 0x20); */
        mov al, 20h;
        out INT_CONTROL, al; /* re-enable interrupts */
    }

    if(rawKey <= 0xD4) {
        if((rawKey & 0x80) == 0x80) {
            unsigned char tKey = (rawKey & 0x7F);
            if(tKey < 84) {
                unsigned char keyIdx = IDX(tKey);
                if(keys[keyIdx] > 0) {
                    keys[keyIdx] = -1;
                }
                else {
                    keys[keyIdx] = 0;
                }
            }
        }
        else {
            if(rawKey < 84) {
                unsigned char keyIdx = IDX(rawKey);
                if(keys[keyIdx] < 1) {
                    keys[keyIdx] = 1;
                }
                else {
                    keys[keyIdx] = 2;
                }
            }
        }
    }
}

void Key_initialize(void) {
    int i;
    oldISR = getvect(KEYBOARD_INT);
    setvect(KEYBOARD_INT, &newKeyInt);

    memset(keys, 0, 84);
}

void Key_terminate(void) {
    setvect(KEYBOARD_INT, oldISR);
}

unsigned char Key_down(unsigned char key) {
    return keys[IDX(key)] > 0;
}

unsigned char Key_pressed(unsigned char key) {
    return keys[IDX(key)] == 1;
}

unsigned char Key_released(unsigned char key) {
    return keys[IDX(key)] == -1;
}

unsigned char Key_up(unsigned char key) {
    return keys[IDX(key)] < 1;
}