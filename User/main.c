#include "anim_frames.h"
#include "bsp_st7789.h"
#include "delay.h"

/*
 * 初始化 ST7789 并循环显示两张 4 位索引测试图片。
 *
 * 每张源图片为 120x120、16 色索引格式，驱动在发送时放大 2 倍覆盖
 * 240x240 屏幕。第一张是暖色斜线和白色数字 1，第二张是冷色棋盘和
 * 黄色数字 2。阶段 2 使用阻塞延时，下一阶段再改为非阻塞动画调度。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 主循环不会返回。
 *
 * 副作用：
 * 初始化 SysTick 延时、GPIOB、SPI2 和 ST7789，并每秒覆盖一次显存。
 */
int main(void)
{
    uint8_t image_index;

    delay_init();
    ST7789_Init();
    image_index = 0U;

    while (1)
    {
        ST7789_ShowIndexed4Image(
            anim_frames[image_index],
            anim_palette,
            ANIM_FRAME_WIDTH,
            ANIM_FRAME_HEIGHT,
            ANIM_PIXEL_SCALE
        );
        delay_ms(ANIM_FRAME_INTERVAL_MS);

        image_index++;
        if (image_index >= ANIM_FRAME_COUNT)
        {
            image_index = 0U;
        }
    }
}
