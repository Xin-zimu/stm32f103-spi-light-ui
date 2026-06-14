# STM32F103 ST7789 两图切换测试工程

当前工程用于验证一个无 CS 引脚的 7 针 ST7789 240x240 彩屏。

上电初始化成功后，屏幕每秒切换一张测试图片：

```text
图片 1：暖色斜线背景、白色数字 1
图片 2：冷色棋盘背景、黄色数字 2
```

两张源图片为120x120、16色4位索引格式，发送时放大2倍覆盖240x240屏幕。
该阶段用于验证图片数组、调色板、索引解码和连续整屏刷新。

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
User/fault_handlers.c
SYSTEM/delay/delay.c
Libraries/src/stm32f10x_gpio.c
Libraries/src/stm32f10x_rcc.c
Libraries/src/stm32f10x_spi.c
```

程序入口是 [User/main.c](User/main.c)，屏幕驱动是
[User/bsp_st7789.c](User/bsp_st7789.c)。

目录中旧 OLED 和实验代码仍然保留，但不参与当前两图切换验收流程。
当前不要运行“一键更换 GIF 并烧录”工具。

## 编译和烧录

使用 [Project/led.uvprojx](Project/led.uvprojx)：

1. 打开 Keil 工程。
2. 执行 `Rebuild`。
3. 确认结果为 `0 Error(s), 0 Warning(s)`。
4. 执行 `Download`。
5. 复位开发板。
6. 确认数字1和数字2每秒稳定交替，没有错色、撕裂或残留。

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

## 后续计划

1. 完成两张图片循环切换验收。
2. 恢复非阻塞动画任务。
3. 适配 GIF 转换数据。

当前阶段验收标准：

1. 图片1与图片2每秒交替。
2. 数字1为白色，数字2为黄色。
3. 图片1为暖色斜线，图片2为蓝青棋盘。
4. 两张图片均覆盖完整240x240可见区。
5. 切换后没有上一张图片的残留像素。
