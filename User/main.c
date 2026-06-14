#include "bsp_st7789.h"
#include "delay.h"

#define COLOR_HOLD_MS             1000U   // 每种测试颜色的显示时间

/*
 * 初始化 ST7789 并循环执行阶段 1 纯色点屏测试。
 *
 * 当前固件只使用 ST7789，不初始化旧 OLED、光敏传感器、交通灯或串口。
 * 红、绿、蓝、白、黑五种颜色用于检查背光、SPI 通信、RGB565 字节顺序
 * 和屏幕可见区域偏移。阶段 1 允许使用阻塞延时，后续动画任务再切换为
 * Timing_GetTick() 驱动的非阻塞调度。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 主循环不会返回。
 *
 * 副作用：
 * 初始化 SysTick 延时、GPIOB、SPI2 和 ST7789，并持续刷新全屏颜色。
 */
int main(void)
{
    static const uint16_t test_colors[] =
    {
        ST7789_RED,
        ST7789_GREEN,
        ST7789_BLUE,
        ST7789_WHITE,
        ST7789_BLACK
    };
    uint8_t color_index;

    delay_init();
    ST7789_Init();
    color_index = 0U;

    while (1)
    {
        ST7789_Clear(test_colors[color_index]);
        delay_ms(COLOR_HOLD_MS);

        color_index++;
        if (color_index >= (uint8_t)(
            sizeof(test_colors) / sizeof(test_colors[0])
        ))
        {
            color_index = 0U;
        }
    }
}
