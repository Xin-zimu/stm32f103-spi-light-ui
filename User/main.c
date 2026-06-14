#include "app_st7789_test.h"
#include "bsp_st7789.h"
#include "delay.h"

/*
 * 初始化 ST7789 并显示阶段 2 方向和颜色诊断画面。
 *
 * 诊断画面使用不对称四角标记、RGB/CMY 色块、中心十字和灰阶块检查
 * 240x320 全屏覆盖、扫描方向、镜像、RGB565 颜色顺序和基本 gamma 表现。
 * 画面只绘制一次，随后保持不变，便于人工观察和逻辑分析仪抓取。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 主循环不会返回。
 *
 * 副作用：
 * 初始化 SysTick 延时、GPIOB、SPI2 和 ST7789，并覆盖整个可见显存。
 */
int main(void)
{
    delay_init();
    ST7789_Init();
    App_ST7789_ShowDiagnosticPattern();

    while (1)
    {
    }
}
