#ifndef TIMER_H
#define TIMER_H

#ifdef TIMER_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void Timer_setTime(void);
EXTERN void Timer_setTimeWithRate(unsigned char updateRate);
EXTERN void Timer_resetTime(void);
EXTERN volatile unsigned long Timer_getFastTick(void);
EXTERN unsigned char Timer_getUpdateRate(void);
EXTERN void Timer_startTimer(void (*onTime)(void), unsigned long tickCount);
EXTERN void Timer_setTimeCritical(void (*timeCriticalCallback)(void));

#undef TIMER_IMPORT
#undef EXTERN

#endif /* TIMER_H */