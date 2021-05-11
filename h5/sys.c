#include <or1k-support.h>
#include <or1k-sprs.h>
#include "io.h"
#include "sys.h"




void enable_cache(void)
{
	unsigned addr;

    for (addr = 0; addr < 16 * 1024 + 32 * 1024; addr += 16)
	{
		or1k_icache_flush(addr);
	}

	or1k_icache_enable();
}

void reset(void)
{
	__asm__("l.j _start");
	__asm__("l.nop");
}

void handle_exception(uint32_t type, uint32_t pc, uint32_t sp)
{
    reset();
}

void clk_set_rate(uint32_t rate)
{
    uint32_t reg;

    if ( rate <= 24000000 ) return;

#if 0
    // if rate <= 432 MHz, the VDD_CPUS/VDD_RTC can be set to 1.1V
    // if rate > 432 MHz, the VDD_CPUS/VDD_RTC must be set to 1.2-1.3V
    if ( rate > 432000000 )
    {
        uint32_t reg_vdd_rtc;
        reg_vdd_rtc = readl(VDD_RTC_REG);
        // 0b100 = 1.1V, 0b101 = 1.2V, 0b110 = 1.3V
        SET_BITS_AT(reg_vdd_rtc, 3, 0, 0b101);
        writel(reg_vdd_rtc, VDD_RTC_REG);
    }

    uint8_t N,K,M,P;

    // the PLL6 output = 24 MHz * (N+1)*(K+1)/(M+1)/(1<<P)/2
    // N: 0..31, K: 0..3, M: 0..3
    if      (rate == 300000000) { N = 24; K = 1; M = 1; P = 0; }
    else if (rate == 312000000) { N = 25; K = 1; M = 1; P = 0; }
    else if (rate == 324000000) { N = 26; K = 1; M = 1; P = 0; }
    else if (rate == 336000000) { N = 27; K = 1; M = 1; P = 0; }
    else if (rate == 348000000) { N = 28; K = 1; M = 1; P = 0; }
    else if (rate == 360000000) { N = 29; K = 1; M = 1; P = 0; }
    else if (rate == 372000000) { N = 30; K = 1; M = 1; P = 0; }
    else if (rate == 384000000) { N = 31; K = 1; M = 1; P = 0; }
    else if (rate == 396000000) { N = 21; K = 2; M = 1; P = 0; }
    else if (rate == 414000000) { N = 22; K = 2; M = 1; P = 0; }
    else if (rate == 432000000) { N = 23; K = 2; M = 1; P = 0; }
    else if (rate == 450000000) { N = 24; K = 2; M = 1; P = 0; }
    else if (rate == 468000000) { N = 25; K = 2; M = 1; P = 0; }
    else if (rate == 486000000) { N = 26; K = 2; M = 1; P = 0; }
    else if (rate == 504000000) { N = 27; K = 2; M = 1; P = 0; }
    else if (rate == 522000000) { N = 28; K = 2; M = 1; P = 0; }
    else if (rate == 540000000) { N = 29; K = 2; M = 1; P = 0; }
    else if (rate == 558000000) { N = 30; K = 2; M = 1; P = 0; }
    else if (rate == 576000000) { N = 31; K = 2; M = 1; P = 0; }
#endif
    reg = readl(PLL6_CTRL_REG);
#if 0
    SET_BITS_AT(reg, 2, 0, M);
    SET_BITS_AT(reg, 2, 4, K);
    SET_BITS_AT(reg, 5, 8, N);
    SET_BITS_AT(reg, 2, 16, P);
#endif
    reg |= PLL6_CTRL_ENABLE | PLL6_CTRL_CLK_OUTEN;
    writel(reg, PLL6_CTRL_REG);

    // Switch AR100 to PLL6
    reg = readl(AR100_CLKCFG_REG);
    reg &= ~AR100_CLKCFG_SRC_MASK;
    reg |= AR100_CLKCFG_SRC_PLL6;
    reg &= ~AR100_CLKCFG_POSTDIV_MASK;
    reg |= AR100_CLKCFG_POSTDIV(1);
    reg &= ~AR100_CLKCFG_DIV_MASK;
    reg |= AR100_CLKCFG_DIV(0);
    writel(reg, AR100_CLKCFG_REG);
}
