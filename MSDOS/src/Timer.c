#include <DOS.h>
#include <TIME.h>

#define TIMER_IMPORT
#include "Timer.h"

#define DEFAULT_RATE    60
#define TIMER           8
#define PIT_FREQ        0x1234DD

typedef struct Timer_t {
    unsigned long endTick;
    unsigned char status;
    void (*callback)(void);
} Timer;

static unsigned char updateRate = DEFAULT_RATE;
static volatile unsigned long fastTick, slowTick;
static void interrupt (*oldTimer)(void);
static volatile Timer timers[8];
static void (*timeCritical)(void);

static void interrupt timer(void) {
    unsigned char i;
    fastTick++;
    if(timeCritical) {
        timeCritical();
    }
    for(i = 0; i<8; ++i) {
        if(timers[i].status != 0) {
            if(fastTick >= timers[i].endTick) {
                timers[i].callback();
                timers[i].status = 0;
            }
        }
    }
    if(!(fastTick & 3)) {
        oldTimer();
        slowTick++;
    }
    else {
        _asm {
            mov al, 0x20;
            out 0x20, al;
        }
    }
}

void Timer_resetTime(void) {
    outportb(0x43, 0x34);
    outportb(0x40, 0);
    outportb(0x40, 0);

    setvect(8, oldTimer);
}

void Timer_setTime(void) {
    Timer_setTimeWithRate(DEFAULT_RATE);
}

void Timer_setTimeWithRate(unsigned char updateRateParam) {
    unsigned long counter = PIT_FREQ / updateRateParam;
    updateRate = updateRateParam;
    slowTick = fastTick = 0l;
    oldTimer = getvect(TIMER);
    setvect(TIMER, &timer);

    outportb(0x43, 0x34);
    outportb(0x40, counter % 256);
    outportb(0x40, counter / 256);
}

void Timer_delayMicroseconds(double msecs) {
    double seconds = msecs * 0.000001, elapsed = 0;
    clock_t start = clock(), current;
    do {
        current = clock();
        elapsed += (double)(current-start)/CLK_TCK;
    } while(elapsed < seconds);
}

volatile unsigned long Timer_getFastTick(void) {
    return fastTick;
}

unsigned char Timer_getUpdateRate(void) {
    return updateRate;
}

void Timer_startTimer(void (*onTime)(void), unsigned long tickCount) {
    unsigned char i;
    for(i=0; i<8; ++i) {
        if(timers[i].status == 0) {
            timers[i].endTick = fastTick + tickCount;
            timers[i].callback = onTime;
            timers[i].status = 1;
            return;
        }
    }
}

void Timer_setTimeCritical(void (*timeCriticalCallback)(void)) {
    timeCritical = timeCriticalCallback;
}