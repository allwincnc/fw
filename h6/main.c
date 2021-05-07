#include "io.h"
#include "sys.h"
#include "gpio.h"
#include "pwm.h"
#include "encoder.h"




int main(void)
{
    enable_cache();
    clk_set_rate(CPU_FREQ);
    pwm_init();
    enc_init();

    for(;;) {
        pwm_main_loop();
        enc_main_loop();
    }

    return 0;
}
