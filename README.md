# STM32F103 ST7789 GIF 动画工程

当前工程用于验证一个无 CS 引脚的 7 针 ST7789 240x240 彩屏。

上电初始化成功后，屏幕循环播放 `小猫图.gif` 转换得到的动画。原 GIF 为
120x120、28 帧、40 ms/帧；工程使用已授权的 Arm Compiler 6.24，
保留全部28帧并按原始40 ms间隔播放，将源像素放大2倍覆盖屏幕。

## 硬件接线

| ST7789 | STM32F103C8T6 | 作用 |
| --- | --- | --- |
| GND | GND | 电源地，必须共地 |
| VCC | 3.3V | 模块电源 |
| SCL | PB13 | SPI2 SCK 时钟 |
| SDA | PB15 | SPI2 MOSI 数据 |
| RES | PB10 | 硬件复位 |
| DC | PB14 | 命令/数据选择 |
| BLK | PB12 | 背光控制，高电平点亮 |

注意：

- `SCL` 和 `SDA` 在该屏幕上是 SPI 信号，不是 I2C。
- 模块没有 MISO，STM32 只向屏幕发送数据。
- 模块没有外部 CS 引脚，代码使用 `ST7789_USE_CS=0`，不依赖片选操作。
- `DC=0` 发送命令，`DC=1` 发送数据。
- 所有信号和电源均按 3.3V 使用。

## 当前驱动配置

驱动配置位于 [User/bsp_st7789.h](User/bsp_st7789.h)：

```c
#define ST7789_WIDTH               240U
#define ST7789_HEIGHT              240U
#define ST7789_X_OFFSET            0U
#define ST7789_Y_OFFSET            0U
#define ST7789_USE_CS              0U
#define ST7789_SPI_MODE            3U
```

当前使用：

- STM32F10x 标准外设库。
- SPI2 主机模式、单线发送、8 位数据、MSB first。
- SPI2 约 4.5 MHz。
- 当前屏幕实测使用 SPI Mode 3。
- RGB565 颜色格式。
- 240x240 地址窗口，Y 偏移为 0。
- 初始化期间先关闭背光，复位并清黑屏后再打开背光。
- 不创建全屏帧缓冲，不使用动态内存。
- TIM3 提供 1 ms 动画时基，等待下一帧期间主循环不阻塞。
- 首帧完整写入，后续帧只更新变化像素。

## SPI Mode 切换

当前实测配置为 Mode 3：

```c
#define ST7789_SPI_MODE            3U
```

如果更换屏幕后只有背光、完全没有纯色，可尝试 Mode 0：

```c
#define ST7789_SPI_MODE            0U
```

修改后必须重新执行 `Rebuild` 和 `Download`。驱动只接受 Mode 0 或 Mode 3，
其他数值会在编译时报告错误。

## 工程文件

当前 Keil 工程主要编译：

```text
User/main.c
User/bsp_st7789.c
User/anim_frames.c
User/app_st7789_anim.c
User/fault_handlers.c
SYSTEM/delay/delay.c
SYSTEM/timing/timing.c
Libraries/src/stm32f10x_gpio.c
Libraries/src/stm32f10x_rcc.c
Libraries/src/stm32f10x_spi.c
Libraries/src/stm32f10x_tim.c
```

程序入口是 [User/main.c](User/main.c)，屏幕驱动是
[User/bsp_st7789.c](User/bsp_st7789.c)。

目录中旧 OLED 和实验代码仍然保留，但不参与当前 GIF 播放流程。

## GIF 转换与存储

[Tools/generate_st7789_gif.py](Tools/generate_st7789_gif.py) 使用 Pillow 完成：

1. 合成 GIF 的透明和局部更新帧。
2. 保留全部28帧动画。
3. 为全部帧建立一套 16 色 RGB565 调色板。
4. 首帧保存为 4 位索引整帧，共 7200 字节。
5. 后续帧按行保存“跳过未变化像素”和“绘制变化像素”指令。
6. 反向解码每个帧差，确认生成数据能精确恢复量化后的图像。

当前帧差数据为49109字节，动画数据合计56309字节。STM32 不在 RAM
中保存上一帧，而是利用 ST7789 显存保留未变化区域。

原始 `小猫图.gif` 是本地输入素材，不纳入 Git；仓库中已包含转换后的
`anim_frames.c/.h`，正常编译和烧录不依赖原始 GIF。

## 编译和烧录

使用 [Project/led.uvprojx](Project/led.uvprojx)：

1. 打开 Keil 工程。
2. 执行 `Rebuild`。
3. 确认结果为 `0 Error(s), 0 Warning(s)`。
4. 执行 `Download`。
5. 复位开发板。
6. 确认小猫动画连续循环，没有错位、闪烁或上一轮残留。

标准输出文件为：

```text
Output/led.hex
```

## 背光亮但没有图像

按以下顺序排查：

1. 确认下载的是当前 `Project/led.uvprojx` 生成的固件。
2. 确认 SCL 接 PB13、SDA 接 PB15，没有按 I2C 方式接线。
3. 确认 RES 接 PB10、DC 接 PB14，二者没有接反。
4. 确认 BLK 接 PB12，且初始化完成后 PB12 为高电平。
5. 测量 PB10，启动时应出现低电平复位脉冲。
6. 测量 PB13，刷屏时应出现 SPI 时钟。
7. 测量 PB15，刷屏时应出现数据变化。
8. 确认 PB14 在命令和数据发送期间会切换电平。
9. 在 Mode 3 和 Mode 0 之间切换后重新编译烧录。
10. 如果信号均正常，确认模块控制器确实是 ST7789，且内部 CS 已固定为有效状态。

背光亮只说明 VCC、GND 和 BLK 基本正常，不代表 ST7789 已经初始化成功。

## 图像偏移或颜色异常

纯色出现但位置不正确时，调整：

```c
#define ST7789_X_OFFSET            0U
#define ST7789_Y_OFFSET            0U
```

本次 240x240 面板实测使用 Y 偏移 0。其他 240x240 模块也可能使用 Y 偏移 80，
必须结合不对称测试图确认，不能只依赖纯色画面。
颜色红蓝互换时，需要继续检查
`ST7789_MADCTL_VALUE` 的 RGB/BGR 配置，但在纯色完全不显示时不要先改颜色方向。

完整排障方法见
[ST7789 与逻辑分析仪排障学习指南](docs/st7789-logic-analyzer-troubleshooting-guide.md)。

当前阶段验收标准：

1. 上电后显示真实小猫动画。
2. 28帧按40 ms间隔连续循环。
3. 动画覆盖完整 240x240 可见区。
4. 帧差更新区域没有错位、花屏或残留。
5. 复位后可重新从首帧开始播放。
