#include "io.h"
#include "sys.h"
#include "gpio.h"
#include "pwm.h"




int main(void)
{
    enable_cache();
    clk_set_rate(CPU_FREQ);
    pwm_init();

    for(;;) pwm_main_loop();

    return 0;
}
