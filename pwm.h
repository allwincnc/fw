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
    PWM_CH_POS_CMD,
    PWM_CH_TICK,
    PWM_CH_TIMEOUT,

    PWM_CH_P_BUSY,
    PWM_CH_P_PORT,
    PWM_CH_P_PIN_MSK,
    PWM_CH_P_PIN_MSKN,
    PWM_CH_P_STATE,
    PWM_CH_P_T0,
    PWM_CH_P_T1,
    PWM_CH_P_STOP,

    PWM_CH_D_BUSY,
    PWM_CH_D_PORT,
    PWM_CH_D_PIN_MSK,
    PWM_CH_D_PIN_MSKN,
    PWM_CH_D_STATE,
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
#define PWM_SHM_SIZE         (PWM_SHM_CH_DATA_BASE + PWM_CH_MAX_CNT*PWM_CH_DATA_CNT*4 - PWM_SHM_BASE)




static volatile uint32_t *pd = (uint32_t*)PWM_SHM_DATA_BASE;
static volatile uint32_t (*pc)[PWM_CH_DATA_CNT] = (void*)PWM_SHM_CH_DATA_BASE;




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
    volatile static uint32_t c;

    // nothing to do? quit
    if ( !pd[PWM_CH_CNT] ) return;

    // is access locked?
    if ( pd[PWM_ARM_LOCK] ) { pd[PWM_ARISC_LOCK] = 0; return; }
    else pd[PWM_ARISC_LOCK] = 1;

    // get timer counter value
    pd[PWM_TIMER_TICK] = TIMER_CNT_GET();

    // check/process channels data
    for ( c = pd[PWM_CH_CNT]; c--; )
    {
        // if PWM output is disabled
        if ( !pc[c][PWM_CH_P_BUSY] ) continue;
        // if not a time to do an action
        if ( (pd[PWM_TIMER_TICK] - pc[c][PWM_CH_TICK]) < pc[c][PWM_CH_TIMEOUT] ) continue;

        if ( pc[c][PWM_CH_D_BUSY] ) { // current task is `direction change`
            pc[c][PWM_CH_D_BUSY] = 0;
            pc[c][PWM_CH_D] = !pc[c][PWM_CH_D];
            pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_D_T1];
            if ( pc[c][PWM_CH_D_STATE] ) { // DIR pin is HIGH
                pc[c][PWM_CH_D_STATE] = 0;
                GPIO_PIN_CLR(pc[c][PWM_CH_D_PORT], pc[c][PWM_CH_D_PIN_MSKN]);
            } else { // DIR pin is LOW
                pc[c][PWM_CH_D_STATE] = 1;
                GPIO_PIN_SET(pc[c][PWM_CH_D_PORT], pc[c][PWM_CH_D_PIN_MSK]);
            }
        } else { // current task is `pwm output`
            if ( pc[c][PWM_CH_P_STATE] ) { // PWM pin is HIGH
                pc[c][PWM_CH_P_STATE] = 0;
                GPIO_PIN_CLR(pc[c][PWM_CH_P_PORT], pc[c][PWM_CH_P_PIN_MSKN]);
                pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_P_T0];
                pc[c][PWM_CH_POS] += pc[c][PWM_CH_D] ? -1 : 1;
                if ( pc[c][PWM_CH_POS] == pc[c][PWM_CH_POS_CMD] ) pc[c][PWM_CH_P_STOP] = 1;
            } else { // PWM pin is LOW
                if ( pc[c][PWM_CH_P_STOP] ) {
                    pc[c][PWM_CH_P_STOP] = 0;
                    pc[c][PWM_CH_P_BUSY] = 0;
                    if ( (c+1) == pd[PWM_CH_CNT] ) { // channels_count--
                        for ( ; c-- && !pc[c][PWM_CH_P_BUSY]; );
                        if ( c >= PWM_CH_MAX_CNT ) { pd[PWM_CH_CNT] = 0; return; }
                        pd[PWM_CH_CNT] = c+1;
                    }
                    continue;
                }
                if ( pc[c][PWM_CH_D_CHANGE] ) {
                    pc[c][PWM_CH_D_CHANGE] = 0;
                    pc[c][PWM_CH_D_BUSY] = 1;
                    pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_D_T0];
                } else {
                    pc[c][PWM_CH_P_STATE] = 1;
                    GPIO_PIN_SET(pc[c][PWM_CH_P_PORT], pc[c][PWM_CH_P_PIN_MSK]);
                    pc[c][PWM_CH_TIMEOUT] = pc[c][PWM_CH_P_T1];
                }
            }
        }

        pc[c][PWM_CH_TICK] = pd[PWM_TIMER_TICK];
    }
}




#endif
