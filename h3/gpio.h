#ifndef _GPIO_H
#define _GPIO_H

#include <stdint.h>
#include "sys.h"




#define GPIO_BASE               0x01c20800
#define GPIO_R_BASE             0x01f02c00
#define GPIO_BANK_SIZE          0x24
#define GPIO_PORTS_MAX_CNT      8
#define GPIO_PINS_MAX_CNT       24

enum { PA, PB, PC, PD, PE, PF, PG, PL };
enum { LOW, HIGH };

#define GPIO_PIN_SET(PORT,PIN_MASK) \
    *gpio[PORT] |= PIN_MASK

#define GPIO_PIN_CLR(PORT,PIN_MASK_NOT) \
    *gpio[PORT] &= PIN_MASK_NOT

#define GPIO_PIN_GET(PORT,PIN_MASK) \
    (*gpio[PORT] & PIN_MASK)




volatile uint32_t *gpio[GPIO_PORTS_MAX_CNT] =
{
    (uint32_t *) ( (GPIO_BASE + PA * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PB * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PC * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PD * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PE * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PF * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_BASE + PG * GPIO_BANK_SIZE) + 16 ),
    (uint32_t *) ( (GPIO_R_BASE                    ) + 16 )
};




#endif
