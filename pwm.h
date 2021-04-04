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

    PWM_CH_P_BUSY,
    PWM_CH_P_PORT,
    PWM_CH_P_PIN_MSK,
    PWM_CH_P_PIN_MSKN,
    PWM_CH_P_T0,
    PWM_CH_P_T1,
    PWM_CH_P_STOP,

    PWM_CH_D_BUSY,
    PWM_CH_D_PORT,
    PWM_CH_D_PIN_MSK,
    PWM_CH_D_PIN_MSKN,
    PWM_CH_D_T0,
    PWM_CH_D_T1,
    PWM_CH_D,
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

#define PWM_SHM_BASE         (ARISC_SHM_BASE)
#define PWM_SHM_DATA_BASE    (PWM_SHM_BASE)
#define PWM_SHM_CH_DATA_BASE (PWM_SHM_DATA_BASE + PWM_DATA_CNT*4)
#define PWM_SHM_SIZE         (PWM_SHM_CH_DATA_BASE + PWM_CH_MAX_CNT*PWM_CH_DATA_CNT*4)




static volatile uint32_t *d = (uint32_t*)PWM_SHM_DATA_BASE;
static volatile uint32_t (*p)[PWM_CH_DATA_CNT] = (void*)PWM_SHM_CH_DATA_BASE;




static inline
void pwm_init()
{
    // cleanup shared memory
    uint32_t i = PWM_SHM_SIZE/4, *p = (uint32_t*)PWM_SHM_BASE;
    for ( ; i--; p++ ) *p = 0;

    // timer setup
    TIMER_START();
}

static inline
void pwm_main_loop()
{
    volatile uint32_t c;

    // is access locked?
    if ( d[PWM_ARM_LOCK] ) { d[PWM_ARISC_LOCK] = 0; return; }
    else d[PWM_ARISC_LOCK] = 1;

    // get timer counter value
    d[PWM_TIMER_TICK] = TIMER_CNT_GET();

    // check/process channels data
    for ( c = d[PWM_CH_CNT]; c--; )
    {
        // nothing to do?
        if ( !p[c][PWM_CH_P_BUSY] ) continue;
        // not a time to do an action?
        if ( (d[PWM_TIMER_TICK] - p[c][PWM_CH_TICK]) < p[c][PWM_CH_TIMEOUT] ) continue;

        if ( p[c][PWM_CH_D_BUSY] ) // current task is `direction change`
        {
            p[c][PWM_CH_D_BUSY] = 0;
            p[c][PWM_CH_D] = !p[c][PWM_CH_D];
            p[c][PWM_CH_TIMEOUT] = p[c][PWM_CH_D_T1];
            if ( GPIO_PIN_GET(p[c][PWM_CH_D_PORT], p[c][PWM_CH_D_PIN_MSK]) ) // clear DIR pin
            {
                GPIO_PIN_CLR(p[c][PWM_CH_D_PORT], p[c][PWM_CH_D_PIN_MSKN]);
            }
            else // set DIR pin
            {
                GPIO_PIN_SET(p[c][PWM_CH_D_PORT], p[c][PWM_CH_D_PIN_MSK]);
            }
        }
        else // current task is `pwm output`
        {
            if ( GPIO_PIN_GET(p[c][PWM_CH_P_PORT], p[c][PWM_CH_P_PIN_MSK]) ) // clear PWM pin
            {
                GPIO_PIN_CLR(p[c][PWM_CH_P_PORT], p[c][PWM_CH_P_PIN_MSKN]);
                p[c][PWM_CH_POS] += p[c][PWM_CH_D] ? -1 : 1;
                if ( p[c][PWM_CH_P_STOP] )
                {
                    p[c][PWM_CH_P_STOP] = 0;
                    p[c][PWM_CH_P_BUSY] = 0;
                    continue;
                }
                if ( p[c][PWM_CH_D_CHANGE] )
                {
                    p[c][PWM_CH_D_CHANGE] = 0;
                    p[c][PWM_CH_D_BUSY] = 1;
                    p[c][PWM_CH_TIMEOUT] = p[c][PWM_CH_D_T0];
                }
                else
                {
                    p[c][PWM_CH_TIMEOUT] = p[c][PWM_CH_P_T0];
                }
            }
            else // set PWM pin
            {
                GPIO_PIN_SET(p[c][PWM_CH_P_PORT], p[c][PWM_CH_P_PIN_MSK]);
                p[c][PWM_CH_TIMEOUT] = p[c][PWM_CH_P_T1];
            }
        }

        p[c][PWM_CH_TICK] = d[PWM_TIMER_TICK];
    }
}




#endif
