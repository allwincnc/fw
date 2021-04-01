#ifndef _PULSGEN_H
#define _PULSGEN_H

#include <stdint.h>
#include "sys.h"
#include "timer.h"
#include "gpio.h"




#define PG_CH_MAX_CNT       24
#define PG_CH_SLOT_MAX_CNT  4
#define PG_CH_SLOT_MAX      (PG_CH_SLOT_MAX_CNT - 1)

enum
{
    PG_OWNER,
    PG_PORT,
    PG_PIN_MSK,
    PG_PIN_MSKN,
    PG_TOGGLES,
    PG_T0,
    PG_T1,
    PG_TICK,
    PG_TIMEOUT,
    PG_CH_DATA_CNT
};

enum
{
    PG_TIMER_TICK,
    PG_ARM_LOCK,
    PG_ARISC_LOCK,
    PG_CH_CNT,
    PG_DATA_CNT
};

enum
{
    PG_OWNER_NONE,
    PG_OWNER_STEPGEN,
    PG_OWNER_PWMGEN
};

#define PG_SHM_BASE         (ARISC_SHM_BASE)
#define PG_SHM_CH_SLOT_BASE (PG_SHM_BASE)
#define PG_SHM_CH_DATA_BASE (PG_SHM_CH_SLOT_BASE + PG_CH_MAX_CNT*4)
#define PG_SHM_DATA_BASE    (PG_SHM_CH_DATA_BASE + PG_CH_MAX_CNT*PG_CH_DATA_CNT*PG_CH_SLOT_MAX_CNT*4)
#define PG_SHM_SIZE         (PG_SHM_DATA_BASE + PG_DATA_CNT*4)




volatile uint32_t *pgs = (void*)PG_SHM_CH_SLOT_BASE;
volatile uint32_t (*pgc)[PG_CH_SLOT_MAX_CNT][PG_CH_DATA_CNT] = (void*)PG_SHM_CH_DATA_BASE;
volatile uint32_t *pgd = (uint32_t*)PG_SHM_DATA_BASE;




static inline
void pulsgen_init()
{
    // cleanup shared memory
    uint32_t i = PG_SHM_SIZE/4, *p = (uint32_t*)PG_SHM_BASE;
    for ( ; i--; p++ ) *p = 0;

    // timer setup
    TIMER_START();
}

static inline
void pulsgen_main_loop()
{
    volatile uint32_t c;

    // is access locked?
    if ( pgd[PG_ARM_LOCK] ) { pgd[PG_ARISC_LOCK] = 0; return; }
    else pgd[PG_ARISC_LOCK] = 1;

    // get timer counter value
    pgd[PG_TIMER_TICK] = TIMER_CNT_GET();

#define sn pgs[c]       // pulsgen channel's slot number
#define sd pgc[c][sn]   // pulsgen channel's slot data

    // check/process channels data
    for ( c = pgd[PG_CH_CNT]; c--; )
    {
        // nothing to do?
        if ( !sd[PG_TOGGLES] ) { sn = (sn + 1) & PG_CH_SLOT_MAX; continue; }
        // not a time to do a pin toggle?
        if ( (pgd[PG_TIMER_TICK] - sd[PG_TICK]) < sd[PG_TIMEOUT] ) continue;

        if ( --sd[PG_TOGGLES] )
        {
            if ( GPIO_PIN_GET(sd[PG_PORT], sd[PG_PIN_MSK]) )
            {
                GPIO_PIN_CLR(sd[PG_PORT], sd[PG_PIN_MSKN]);
                sd[PG_TIMEOUT] = sd[PG_T0];
            }
            else
            {
                GPIO_PIN_SET(sd[PG_PORT], sd[PG_PIN_MSK]);
                sd[PG_TIMEOUT] = sd[PG_T1];
            }
        }
        else sn = (sn + 1) & PG_CH_SLOT_MAX;

        sd[PG_TICK] = pgd[PG_TIMER_TICK];
    }

#undef sn
#undef sd
}




#endif
