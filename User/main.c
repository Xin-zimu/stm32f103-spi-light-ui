#include "stm32f10x.h"
#include "delay.h"
#include "timing.h"

#define SPI_OLED_ANIM_DEMO        1U      // 1：SPI 动画；0：原 I2C 光敏界面

#if SPI_OLED_ANIM_DEMO
#include "app_anim.h"
#include "bsp_spi_oled.h"
#else
#include "led.h"
#include "light_sensor.h"
#include "oled.h"
#endif

/*
 * 初始化选定的显示模式，并运行裸机轮询主循环。
 *
 * SPI_OLED_ANIM_DEMO 用于选择新的全屏 SPI 动画演示；设为 0 时恢复原有
 * 光敏传感器和 I2C OLED 逻辑，避免两个显示模块同时刷新屏幕。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 固件主循环不会返回。
 *
 * 副作用：
 * 初始化定时器以及当前模式使用的外设。
 */
int main(void)
{
#if SPI_OLED_ANIM_DEMO
    delay_init();
    Timing_Init();
    OLED_SPI_PanelInit();
    App_Anim_Init();

    while (1)
    {
        App_Anim_Task();
    }
#else
    uint32_t last_time = 0;
    uint16_t light_value;

    LED_Init();
    Timing_Init();
    LightSensor_Init();
    LightSensor_ADC_Init();
    OLED_Init();

    OLED_Clear();
    OLED_ShowString(0, 0, "Light:");

    while (1)
    {
        if (Timing_GetTick() - last_time >= 200)
        {
            last_time = Timing_GetTick();

            light_value = LightSensor_ReadAO();

            OLED_ShowString(4, 0, "     ");
            OLED_ShowNum(4, 0, light_value, 4);

            if (LightSensor_IsDark())
            {
                Traffic_RedOn();
                OLED_ShowString(2, 0, "Dark  ");
            }
            else
            {
                Traffic_GreenOn();
                OLED_ShowString(2, 0, "Bright");
            }
        }
    }
#endif
}
