#include "app_st7789_anim.h"
#include "app_ui.h"
#include "bsp_st7789.h"
#include "delay.h"
#include "key_driver.h"
#include "timing.h"

/*
 * Initialize the board and continuously service the lightweight UI system.
 *
 * SysTick remains dedicated to the display driver's startup delays, while TIM3
 * supplies the millisecond scheduler used by key scanning and GIF playback.
 * PA0 through PA6 are configured as common-ground joystick inputs, and the UI
 * task decides when to enter the existing ST7789 GIF player.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * The firmware main loop does not return.
 *
 * Side effects:
 * Configures timing hardware, GPIOA keys, SPI2, GPIOB, and ST7789.
 */
int main(void)
{
    delay_init();
    Timing_Init();
    ST7789_Init();
    Key_Init();
    App_UI_Init();

    while (1)
    {
        uint32_t now;

        now = Timing_GetTick();
        Key_Task(now);
        App_UI_Task(now);
    }
}
