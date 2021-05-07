#ifndef _SYS_H
#define _SYS_H

#include <stdint.h>




#define CPU_FREQ                450000000 // Hz

#define ARISC_FW_BASE           (0x00000000) // for ARM CPU it's 0x00040000
#define ARISC_FW_SIZE           ((8+8+32)*1024)
#define ARISC_SHM_SIZE          (4096)
#define ARISC_SHM_BASE          (ARISC_FW_BASE + ARISC_FW_SIZE - ARISC_SHM_SIZE)

/* ccm */
#define CCM_BASE                0x01c20000
#define R_PRCM_BASE             0x01f01400

/* pll6 */
#define PLL6_CTRL_REG           (CCM_BASE + 0x0028)
#define PLL6_CTRL_ENABLE        BIT(31)
#define PLL6_CTRL_LOCK          BIT(28)
#define PLL6_CTRL_BYPASS        BIT(25)
#define PLL6_CTRL_CLK_OUTEN     BIT(24)
#define PLL6_CTRL_24M_OUTEN     BIT(18)

/*
 * AR100 clock configuration register:
 * [31:18] Reserved
 * [17:16] Clock source (00: LOSC, 01: HOSC, 10/11: PLL6/PDIV)
 * [15:13] Reserved
 * [12:8]  Post divide (00000: 1 - 11111: 32)
 * [7:6]   Reserved
 * [5:4]   Clock divide ratio (00: 1, 01: 2, 10: 4, 11: 8)
 * [3:0]   Reserved
 */
#define AR100_CLKCFG_REG            (R_PRCM_BASE + 0x000)
#define AR100_CLKCFG_SRC_LOSC       (0 << 16)
#define AR100_CLKCFG_SRC_HOSC       (1 << 16)
#define AR100_CLKCFG_SRC_PLL6       (2 << 16)
#define AR100_CLKCFG_SRC_MASK       (0x3 << 16)
#define AR100_CLKCFG_POSTDIV(x)     (((x) & 0x1f) << 8)
#define AR100_CLKCFG_POSTDIV_MASK   (0x1f << 8)
#define AR100_CLKCFG_DIV(x)         (((x) & 0x3) << 4)
#define AR100_CLKCFG_DIV_MASK       (0x3 << 4)

// ARISC/CPUS and RTC power regulation
#define VDD_RTC_REG                 0x01f00190




void enable_cache(void);
void reset(void);
void handle_exception(uint32_t type, uint32_t pc, uint32_t sp);
void clk_set_rate(uint32_t rate);




#endif
