#ifndef _PWM_H
#define _PWM_H

#include <stdint.h>
#include "sys.h"
#include "timer.h"
#include "gpio.h"




#define PWM_CH_MAX_CNT 16

enum
{
    PWM_CH_POS,
    PWM_CH_TICK,
    PWM_CH_TIMEOUT,
    PWM_CH_STATE,

    PWM_CH_P_PORT,
    PWM_CH_P_PIN_MSK,
    PWM_CH_P_PIN_MSKN,
    PWM_CH_P_INV,
    PWM_CH_P_T0,
    PWM_CH_P_T1,
    PWM_CH_P_STOP,

    PWM_CH_D_PORT,
    PWM_CH_D_PIN_MSK,
    PWM_CH_D_PIN_MSKN,
    PWM_CH_D,
    PWM_CH_D_INV,
    PWM_CH_D_T0,
    PWM_CH_D_T1,
    PWM_CH_D_CHANGE,

    PWM_CH_DATA_CNT
};

enum
{
    PWM_TIMER_TICK,
    PWM_ARM_LOCK,
    PWM_ARISC_LOCK,
    PWM_CH_CNT,
    PWM_DATA_CNT
};

enum
{
    PWM_CH_STATE_IDLE,
    PWM_CH_STATE_P0,
    PWM_CH_STATE_P1,
    PWM_CH_STATE_D0,
    PWM_CH_STATE_D1
};

#define PWM_SHM_BASE         (ARISC_SHM_BASE)
#define PWM_SHM_DATA_BASE    (PWM_SHM_BASE)
#define PWM_SHM_CH_DATA_BASE (PWM_SHM_DATA_BASE + PWM_DATA_CNT*4)
#define PWM_SHM_SIZE         (PWM_SHM_CH_DATA_BASE + PWM_CH_MAX_CNT*PWM_CH_DATA_CNT*4 - PWM_SHM_BASE)

#define PWM_P_PIN_SET() \
    if ( pc[c][PWM_CH_P_INV] ) GPIO_PIN_CLR(pc[c][PWM_CH_P_PORT], pc[c][PWM_CH_P_PIN_MSKN]); \
    else                       GPIO_PIN_SET(pc[c][PWM_CH_P_PORT], pc[c][PWM_CH_P_PIN_MSK])
#define PWM_P_PIN_CLR() \
    if ( pc[c][PWM_CH_P_INV] ) GPIO_PIN_SET(pc[c][PWM_CH_P_PORT], pc[c][PWM_CH_P_PIN_MSK]); \
    else                       GPIO_PIN_CLR(pc[c][PWM_CH_P_PORT], pc[c][PWM_CH_P_PIN_MSKN])
#define PWM_D_PIN_SET() \
    if ( pc[c][PWM_CH_D_INV] ) GPIO_PIN_CLR(pc[c][PWM_CH_D_PORT], pc[c][PWM_CH_D_PIN_MSKN]); \
    else                       GPIO_PIN_SET(pc[c][PWM_CH_D_PORT], pc[c][PWM_CH_D_PIN_MSK])
#define PWM_D_PIN_CLR() \
    if ( pc[c][PWM_CH_D_INV] ) GPIO_PIN_SET(pc[c][PWM_CH_D_PORT], pc[c][PWM_CH_D_PIN_MSK]); \
    else                       GPIO_PIN_CLR(pc[c][PWM_CH_D_PORT], pc[c][PWM_CH_D_PIN_MSKN])




static volatile uint32_t *pd = (uint32_t*)PWM_SHM_DATA_BASE;
static volatile uint32_t (*pc)[PWM_CH_DATA_CNT] = (void*)PWM_SHM_CH_DATA_BASE;




static inline
void pwm_init()
{
    // cleanup shared memory
    uint32_t i = PWM_SHM_SIZE/4, *p = (uint32_t*)PWM_SHM_BASE;
    for ( ; i--; p++ ) *p = 0;

    TIMER_START();
}

static inline
void pwm_main_loop()
{
    volatile static uint32_t c;

    if ( !pd[PWM_CH_CNT] ) return;

    if ( pd[PWM_ARM_LOCK] ) { pd[PWM_ARISC_LOCK] = 0; return; }
    else pd[PWM_ARISC_LOCK] = 1;

    pd[PWM_TIMER_TICK] = TIMER_CNT_GET();

    for ( c = pd[PWM_CH_CNT]; c--; )
    {
        if ( !pc[c][PWM_CH_STATE] ) continue;
        if ( (pd[PWM_TIMER_TICK] - pc[c][PWM_CH_TICK]) < pc[c][PWM_CH_TIMEOUT] ) continue;

        switch ( pc[c][PWM_CH_STATE] ) {
            case PWM_CH_STATE_P0: {
                P0:
                if ( pc[c][PWM_CH_P_STOP] ) {
                    P_STOP:
                    pc[c][PWM_CH_STATE] = PWM_CH_STATE_IDLE;
                    pc[c][PWM_CH_TIMEOUT] = 0;
                    pc[c][PWM_CH_P_STOP] = 0;
                } else if ( pc[c][PWM_CH_D_CHANGE] ) {
                    D_CHANGE:
                    pc[c][PWM_CH_STATE] = PWM_CH_STATE_D0;
                    pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_D_T0];
                    pc[c][PWM_CH_D_CHANGE] = 0;
                } else {
                    PWM_P_PIN_SET();
                    pc[c][PWM_CH_STATE] = PWM_CH_STATE_P1;
                    pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_P_T1];
                    pc[c][PWM_CH_POS] += pc[c][PWM_CH_D] ? -1 : 1;
                }
                break;
            }
            case PWM_CH_STATE_P1: {
                PWM_P_PIN_CLR();
                if ( pc[c][PWM_CH_P_STOP] ) goto P_STOP;
                if ( pc[c][PWM_CH_D_CHANGE] ) goto D_CHANGE;
                pc[c][PWM_CH_STATE] = PWM_CH_STATE_P0;
                pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_P_T0];
                break;
            }
            case PWM_CH_STATE_D0: {
                pc[c][PWM_CH_D] = !pc[c][PWM_CH_D];
                if ( pc[c][PWM_CH_D] ) PWM_D_PIN_SET(); else PWM_D_PIN_CLR();
                pc[c][PWM_CH_STATE] = PWM_CH_STATE_D1;
                pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_D_T1];
                break;
            }
            case PWM_CH_STATE_D1: goto P0;
        }

        pc[c][PWM_CH_TICK] = pd[PWM_TIMER_TICK];
    }
}




#endif
