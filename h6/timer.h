/**
 * @file    timer.h
 *
 * @brief   system timer control header
 *
 * @note    timer frequency (TIMER_FREQUENCY) is same as CPU frequency
 *
 * These macros implements an API to the system timer
 */

#ifndef _TIMER_H
#define _TIMER_H

#include <or1k-sprs.h>
#include <or1k-support.h>
#include <stdint.h>
#include "sys.h"




// public macros

/// the system timer frequency in Hz (same as CPU frequency)
#define TIMER_FREQUENCY CPU_FREQ

#define TIMER_START() \
    or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, OR1K_SPR_TICK_TTMR_MODE_SET(0, OR1K_SPR_TICK_TTMR_MODE_CONTINUE))
#define TIMER_STOP() \
    or1k_mtspr(OR1K_SPR_TICK_TTMR_ADDR, 0)
#define TIMER_CNT_SET(CNT) \
    or1k_mtspr(OR1K_SPR_TICK_TTCR_ADDR, CNT)
#define TIMER_CNT_GET() \
    or1k_mfspr(OR1K_SPR_TICK_TTCR_ADDR)




#endif
