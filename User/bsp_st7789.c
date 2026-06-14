#include "bsp_st7789.h"
#include "delay.h"

#define ST7789_GPIO_PORT            GPIOB
#define ST7789_GPIO_CLK             RCC_APB2Periph_GPIOB
#define ST7789_SPI_CLK              RCC_APB1Periph_SPI2

#define ST7789_SCK_PIN              GPIO_Pin_13
#define ST7789_MOSI_PIN             GPIO_Pin_15
#define ST7789_RST_PIN              GPIO_Pin_10
#define ST7789_DC_PIN               GPIO_Pin_14
#define ST7789_BLK_PIN              GPIO_Pin_12

#define ST7789_RST_HIGH()           GPIO_SetBits(ST7789_GPIO_PORT, ST7789_RST_PIN)
#define ST7789_RST_LOW()            GPIO_ResetBits(ST7789_GPIO_PORT, ST7789_RST_PIN)
#define ST7789_DC_HIGH()            GPIO_SetBits(ST7789_GPIO_PORT, ST7789_DC_PIN)
#define ST7789_DC_LOW()             GPIO_ResetBits(ST7789_GPIO_PORT, ST7789_DC_PIN)
#define ST7789_BLK_HIGH()           GPIO_SetBits(ST7789_GPIO_PORT, ST7789_BLK_PIN)
#define ST7789_BLK_LOW()            GPIO_ResetBits(ST7789_GPIO_PORT, ST7789_BLK_PIN)

/*
 * 初始化 ST7789 使用的 GPIOB 引脚。
 *
 * PB13 和 PB15 配置为 SPI2 复用推挽输出，分别连接屏幕 SCL 和 SDA。
 * PB10、PB14、PB12 配置为普通推挽输出，分别控制复位、命令数据选择和
 * 背光。初始化期间先关闭背光，避免控制器显存尚未清空时出现闪屏。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 打开 GPIOB 时钟并改变 PB10、PB12、PB13、PB14、PB15 的工作模式。
 */
void ST7789_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(ST7789_GPIO_CLK, ENABLE);

    gpio.GPIO_Pin = ST7789_SCK_PIN | ST7789_MOSI_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(ST7789_GPIO_PORT, &gpio);

    gpio.GPIO_Pin = ST7789_RST_PIN | ST7789_DC_PIN | ST7789_BLK_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ST7789_GPIO_PORT, &gpio);

    ST7789_RST_HIGH();
    ST7789_DC_HIGH();
    ST7789_BLK_LOW();
}

/*
 * 初始化用于 ST7789 单向发送的硬件 SPI2。
 *
 * SPI2 配置为主机、8 位、MSB 先发送、软件 NSS 和单线发送。时钟极性
 * 与采样边沿由 ST7789_SPI_MODE 选择，当前实测使用 Mode 3；若更换屏幕
 * 模块，可改为 Mode 0 复测。APB1 为 36 MHz 时，8 分频得到约 4.5 MHz 时钟。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 打开并重新配置 SPI2 外设。
 */
void ST7789_SPI_Init(void)
{
    SPI_InitTypeDef spi;

    RCC_APB1PeriphClockCmd(ST7789_SPI_CLK, ENABLE);
    SPI_I2S_DeInit(SPI2);

    spi.SPI_Direction = SPI_Direction_1Line_Tx;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
#if ST7789_SPI_MODE == 0U
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
#elif ST7789_SPI_MODE == 3U
    spi.SPI_CPOL = SPI_CPOL_High;
    spi.SPI_CPHA = SPI_CPHA_2Edge;
#else
#error "ST7789_SPI_MODE must be 0 or 3"
#endif
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7U;

    SPI_Init(SPI2, &spi);
    SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);
    SPI_Cmd(SPI2, ENABLE);
}

/*
 * 通过 SPI2 发送一个字节。
 *
 * 函数先等待发送寄存器为空，再写入数据，并等待 SPI2 完成最后一个时钟。
 * 屏幕没有 MISO 和 CS，因此这里只执行单向发送，也不切换片选信号。
 *
 * 参数：
 * data：需要发送的 8 位数据。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 在 PB13 和 PB15 上产生一次由 ST7789_SPI_MODE 指定的字节传输。
 */
void ST7789_WriteByte(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
    {
    }

    SPI_I2S_SendData(SPI2, data);

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET)
    {
    }
}

/*
 * 向 ST7789 写入一个命令字节。
 *
 * 发送前把 DC 拉低，传输结束后保持当前电平。七针模块没有 CS 引脚，
 * 默认模块内部已经保持选中状态。
 *
 * 参数：
 * cmd：ST7789 命令码。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 改变 PB14 电平并发送一个 SPI 字节。
 */
void ST7789_WriteCmd(uint8_t cmd)
{
    ST7789_DC_LOW();
    ST7789_WriteByte(cmd);
}

/*
 * 向 ST7789 写入一个 8 位参数或显存数据。
 *
 * 参数：
 * data：需要发送的数据。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 把 PB14 拉高并发送一个 SPI 字节。
 */
void ST7789_WriteData8(uint8_t data)
{
    ST7789_DC_HIGH();
    ST7789_WriteByte(data);
}

/*
 * 按 ST7789 要求的大端顺序写入一个 RGB565 数据。
 *
 * RGB565 的高字节必须先发送，低字节后发送。若颜色红蓝颠倒，应优先
 * 检查 MADCTL 的 BGR 位，而不是交换这两个字节。
 *
 * 参数：
 * data：16 位寄存器参数或 RGB565 像素值。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 连续发送两个 SPI 数据字节。
 */
void ST7789_WriteData16(uint16_t data)
{
    ST7789_WriteData8((uint8_t)(data >> 8));
    ST7789_WriteData8((uint8_t)data);
}

/*
 * 执行 ST7789 硬件复位时序。
 *
 * 复位脚先保持高电平，再产生不少于 10 ms 的低脉冲，最后等待控制器
 * 从复位状态恢复。调用本函数前必须已经执行 delay_init()。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 翻转 PB10，并阻塞约 150 ms。
 */
void ST7789_Reset(void)
{
    ST7789_RST_HIGH();
    delay_ms(10U);
    ST7789_RST_LOW();
    delay_ms(20U);
    ST7789_RST_HIGH();
    delay_ms(120U);
}

/*
 * 打开 ST7789 模块背光。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 将 PB12 输出高电平。
 */
void ST7789_Backlight_On(void)
{
    ST7789_BLK_HIGH();
}

/*
 * 关闭 ST7789 模块背光。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 将 PB12 输出低电平。
 */
void ST7789_Backlight_Off(void)
{
    ST7789_BLK_LOW();
}

/*
 * 设置下一次显存写入使用的矩形地址窗口。
 *
 * 输入坐标使用 240x320 可见区域坐标。函数在发送 CASET 和 RASET 前
 * 自动叠加面板偏移，并在最后发送 RAMWR。若图像整体偏移，应修改头文件
 * 中的 ST7789_X_OFFSET 或 ST7789_Y_OFFSET。
 *
 * 参数：
 * x0：窗口左上角 X 坐标。
 * y0：窗口左上角 Y 坐标。
 * x1：窗口右下角 X 坐标，包含该列。
 * y1：窗口右下角 Y 坐标，包含该行。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 更新 ST7789 的列地址、行地址，并进入显存写入状态。
 */
void ST7789_SetAddressWindow(
    uint16_t x0,
    uint16_t y0,
    uint16_t x1,
    uint16_t y1
)
{
    x0 = (uint16_t)(x0 + ST7789_X_OFFSET);
    x1 = (uint16_t)(x1 + ST7789_X_OFFSET);
    y0 = (uint16_t)(y0 + ST7789_Y_OFFSET);
    y1 = (uint16_t)(y1 + ST7789_Y_OFFSET);

    ST7789_WriteCmd(0x2AU);
    ST7789_WriteData16(x0);
    ST7789_WriteData16(x1);

    ST7789_WriteCmd(0x2BU);
    ST7789_WriteData16(y0);
    ST7789_WriteData16(y1);

    ST7789_WriteCmd(0x2CU);
}

/*
 * 使用一个 RGB565 颜色填充整个 240x320 可见区域。
 *
 * 像素直接从常量变量流式发送到屏幕，不创建全屏缓冲区，因此只占用
 * 少量栈空间，不消耗 153600 字节 RAM。
 *
 * 参数：
 * color：需要填充的 RGB565 颜色。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 覆盖屏幕整个可见区域。
 */
void ST7789_Clear(uint16_t color)
{
    uint32_t pixel_count;

    ST7789_SetAddressWindow(
        0U,
        0U,
        ST7789_WIDTH - 1U,
        ST7789_HEIGHT - 1U
    );

    ST7789_DC_HIGH();
    for (pixel_count = 0U;
         pixel_count < (uint32_t)ST7789_WIDTH * ST7789_HEIGHT;
         pixel_count++)
    {
        ST7789_WriteByte((uint8_t)(color >> 8));
        ST7789_WriteByte((uint8_t)color);
    }
}

/*
 * 使用单一 RGB565 颜色填充可见区域中的矩形。
 *
 * 宽度或高度为 0、起点位于屏幕外时不执行写入。矩形越过右边界或下边界时
 * 自动裁剪，防止地址窗口超过当前 240x320 可见区域。像素直接流式发送，
 * 不申请矩形缓冲区。
 *
 * 参数：
 * x：矩形左上角 X 坐标。
 * y：矩形左上角 Y 坐标。
 * width：矩形宽度，单位为像素。
 * height：矩形高度，单位为像素。
 * color：矩形填充使用的 RGB565 颜色。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 覆盖指定矩形范围内的 ST7789 显存。
 */
void ST7789_FillRect(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    uint16_t color
)
{
    uint32_t pixel_count;

    if ((width == 0U) || (height == 0U) ||
        (x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
    {
        return;
    }

    if (width > (uint16_t)(ST7789_WIDTH - x))
    {
        width = (uint16_t)(ST7789_WIDTH - x);
    }
    if (height > (uint16_t)(ST7789_HEIGHT - y))
    {
        height = (uint16_t)(ST7789_HEIGHT - y);
    }

    ST7789_SetAddressWindow(
        x,
        y,
        (uint16_t)(x + width - 1U),
        (uint16_t)(y + height - 1U)
    );

    ST7789_DC_HIGH();
    for (pixel_count = 0U;
         pixel_count < (uint32_t)width * height;
         pixel_count++)
    {
        ST7789_WriteByte((uint8_t)(color >> 8));
        ST7789_WriteByte((uint8_t)color);
    }
}

/*
 * 初始化 240x320 RGB565 ST7789 屏幕。
 *
 * 流程依次完成 GPIO、SPI2、硬件复位、软件复位、退出睡眠、扫描方向、
 * RGB565 色深、电源和 gamma 参数配置，然后打开显示。背光在清黑屏后
 * 才打开，防止上电时显示随机显存内容。
 *
 * 参数：
 * 无。
 *
 * 返回值：
 * 无。
 *
 * 副作用：
 * 配置 GPIOB 和 SPI2，复位显示控制器，清空屏幕并打开背光。
 */
void ST7789_Init(void)
{
    static const uint8_t gamma_positive[14] =
    {
        0xD0U, 0x04U, 0x0DU, 0x11U, 0x13U, 0x2BU, 0x3FU,
        0x54U, 0x4CU, 0x18U, 0x0DU, 0x0BU, 0x1FU, 0x23U
    };
    static const uint8_t gamma_negative[14] =
    {
        0xD0U, 0x04U, 0x0CU, 0x11U, 0x13U, 0x2CU, 0x3FU,
        0x44U, 0x51U, 0x2FU, 0x1FU, 0x1FU, 0x20U, 0x23U
    };
    uint8_t index;

    ST7789_GPIO_Init();
    ST7789_SPI_Init();
    ST7789_Backlight_Off();
    ST7789_Reset();

    ST7789_WriteCmd(0x01U);
    delay_ms(150U);

    ST7789_WriteCmd(0x11U);
    delay_ms(120U);

    ST7789_WriteCmd(0x36U);
    ST7789_WriteData8(ST7789_MADCTL_VALUE);

    ST7789_WriteCmd(0x3AU);
    ST7789_WriteData8(0x55U);

    ST7789_WriteCmd(0xB2U);
    ST7789_WriteData8(0x0CU);
    ST7789_WriteData8(0x0CU);
    ST7789_WriteData8(0x00U);
    ST7789_WriteData8(0x33U);
    ST7789_WriteData8(0x33U);

    ST7789_WriteCmd(0xB7U);
    ST7789_WriteData8(0x35U);
    ST7789_WriteCmd(0xBBU);
    ST7789_WriteData8(0x19U);
    ST7789_WriteCmd(0xC0U);
    ST7789_WriteData8(0x2CU);
    ST7789_WriteCmd(0xC2U);
    ST7789_WriteData8(0x01U);
    ST7789_WriteCmd(0xC3U);
    ST7789_WriteData8(0x12U);
    ST7789_WriteCmd(0xC4U);
    ST7789_WriteData8(0x20U);
    ST7789_WriteCmd(0xC6U);
    ST7789_WriteData8(0x0FU);

    ST7789_WriteCmd(0xD0U);
    ST7789_WriteData8(0xA4U);
    ST7789_WriteData8(0xA1U);

    ST7789_WriteCmd(0xE0U);
    for (index = 0U; index < 14U; index++)
    {
        ST7789_WriteData8(gamma_positive[index]);
    }

    ST7789_WriteCmd(0xE1U);
    for (index = 0U; index < 14U; index++)
    {
        ST7789_WriteData8(gamma_negative[index]);
    }

    ST7789_WriteCmd(0x21U);
    ST7789_WriteCmd(0x13U);
    ST7789_WriteCmd(0x29U);
    delay_ms(20U);

    ST7789_Clear(ST7789_BLACK);
    ST7789_Backlight_On();
}
