#ifndef _ENCODER_H
#define _ENCODER_H

#include <stdint.h>
#include "sys.h"
#include "gpio.h"
#include "pwm.h"




#define ENC_CH_MAX_CNT 8

enum
{
    ENC_CH_BUSY,
    ENC_CH_POS,
    ENC_CH_AB_STATE,

    ENC_CH_A_PORT,
    ENC_CH_A_PIN_MSK,
    ENC_CH_A_INV,
    ENC_CH_A_ALL,
    ENC_CH_A_STATE,

    ENC_CH_B_USE,
    ENC_CH_B_PORT,
    ENC_CH_B_PIN_MSK,
    ENC_CH_B_STATE,

    ENC_CH_Z_USE,
    ENC_CH_Z_PORT,
    ENC_CH_Z_PIN_MSK,
    ENC_CH_Z_INV,
    ENC_CH_Z_ALL,
    ENC_CH_Z_STATE,

    ENC_CH_DATA_CNT
};

enum
{
    ENC_ARM_LOCK,
    ENC_ARISC_LOCK,
    ENC_CH_CNT,
    ENC_DATA_CNT
};

#define ENC_SHM_BASE         (PWM_SHM_BASE + PWM_SHM_SIZE)
#define ENC_SHM_DATA_BASE    (ENC_SHM_BASE)
#define ENC_SHM_CH_DATA_BASE (ENC_SHM_DATA_BASE + ENC_DATA_CNT*4)
#define ENC_SHM_SIZE         (ENC_SHM_CH_DATA_BASE + ENC_CH_MAX_CNT*ENC_CH_DATA_CNT*4 - ENC_SHM_BASE)




static const uint32_t es[4] =
{
    //      clockwise (CW) direction phase states sequence

    //      AB AB AB AB AB AB AB AB AB AB AB AB AB
    //      00 01 11 10 00 01 11 10 00 01 11 10 00

    // A    _____|`````|_____|`````|_____|`````|__

    // B    __|`````|_____|`````|_____|`````|_____

    //        AB    AB    AB    AB
    //      0b00, 0b01, 0b11, 0b10

    // prev    next
    //   AB      AB
    /* 0b00 */ 0b01,
    /* 0b01 */ 0b11,
    /* 0b10 */ 0b00,
    /* 0b11 */ 0b10
};
static volatile uint32_t *ed = (uint32_t*)ENC_SHM_DATA_BASE;
static volatile uint32_t (*ec)[ENC_CH_DATA_CNT] = (void*)ENC_SHM_CH_DATA_BASE;




static inline
void enc_init()
{
    // cleanup shared memory
    uint32_t i = ENC_SHM_SIZE/4, *p = (uint32_t*)ENC_SHM_BASE;
    for ( ; i--; p++ ) *p = 0;
}

static inline
void enc_main_loop()
{
    volatile static uint32_t c, A, B, Z, AB;

    // nothing to do? quit
    if ( !ed[ENC_CH_CNT] ) return;

    // is access locked?
    if ( ed[ENC_ARM_LOCK] ) { ed[ENC_ARISC_LOCK] = 0; return; }
    else ed[ENC_ARISC_LOCK] = 1;

    // check/process channels data
    for ( c = ed[ENC_CH_CNT]; c--; )
    {
        // if channel is disabled
        if ( !ec[c][ENC_CH_BUSY] ) {
            if ( (c+1) == ed[ENC_CH_CNT] ) { // channels_count--
                for ( ; c-- && !ec[c][ENC_CH_BUSY]; );
                if ( c >= ENC_CH_MAX_CNT ) { ed[ENC_CH_CNT] = 0; return; }
                ed[ENC_CH_CNT] = c+1;
            }
            continue;
        }

        // index (Z) searching is active
        if ( ec[c][ENC_CH_Z_USE] ) {
            Z = GPIO_PIN_GET(ec[c][ENC_CH_Z_PORT], ec[c][ENC_CH_Z_PIN_MSK]);
            if ( ec[c][ENC_CH_Z_STATE] != Z ) {
                ec[c][ENC_CH_Z_STATE] = Z;
                if ( ec[c][ENC_CH_Z_ALL] ||
                     (Z && !ec[c][ENC_CH_Z_INV]) ||
                     (!Z && ec[c][ENC_CH_Z_INV])
                ) {
                    ec[c][ENC_CH_Z_USE] = 0;
                    ec[c][ENC_CH_POS] = 0;
                }
            }
        }

        A = GPIO_PIN_GET(ec[c][ENC_CH_A_PORT], ec[c][ENC_CH_A_PIN_MSK]);

        // we are using two phazes (AB) encoder
        if ( ec[c][ENC_CH_B_USE] ) {
            B = GPIO_PIN_GET(ec[c][ENC_CH_B_PORT], ec[c][ENC_CH_B_PIN_MSK]);
            if ( ec[c][ENC_CH_A_STATE] != A || ec[c][ENC_CH_B_STATE] != B ) {
                AB = (A ? 0b10 : 0) | (B ? 0b01 : 0); // get encoder state
                ec[c][ENC_CH_POS] += es[ ec[c][ENC_CH_AB_STATE] ] == AB ? 1 : -1;
                ec[c][ENC_CH_AB_STATE] = AB;
            }
            ec[c][ENC_CH_B_STATE] = B;
        }
        // we are using single phaze encoder AND it's state was changed
        else if ( ec[c][ENC_CH_A_STATE] != A ) {
            if ( ec[c][ENC_CH_Z_ALL] ||
                 (A && !ec[c][ENC_CH_A_INV]) ||
                 (!A && ec[c][ENC_CH_A_INV]) ) ec[c][ENC_CH_POS]++;
        }

        ec[c][ENC_CH_A_STATE] = A;
    }
}




#endif
