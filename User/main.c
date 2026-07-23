#include "app_st7789_anim.h"
#include "bsp_st7789.h"
#include "delay.h"
#include "timing.h"
#include "usart.h"

/*
 * Initialize the ST7789 and continuously service non-blocking GIF playback.
 *
 * SysTick remains dedicated to the display driver's startup delays, while
 * TIM3 supplies the millisecond scheduler used between animation frames.
 * Display commands and address windows still use synchronous SPI bytes, while
 * pixel streams use SPI2 TX DMA. Delta-frame playback advances through a
 * cooperative task and returns while DMA is moving pixel data.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * The firmware main loop does not return.
 *
 * Side effects:
 * Configures timing hardware, USART1, SPI2, GPIOB, and the ST7789 display.
 */
int main(void)
{
    delay_init();
    Timing_Init();
    uart_init(115200);
    ST7789_Init();
    App_ST7789_AnimInit();

    while (1)
    {
        App_ST7789_AnimTask();
    }
}
