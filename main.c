#include "io.h"
#include "sys.h"
#include "gpio.h"
#include "pulsgen.h"




int main(void)
{
    enable_cache();
    clk_set_rate(CPU_FREQ);
    pulsgen_init();

    for(;;) pulsgen_main_loop();

    return 0;
}
