# STM32F103 SPI OLED GIF 动画

## 1. 实现范围

本工程保留原有 `User/oled.c`、`User/oled.h` 软件 I2C OLED 驱动，并新增独立的 SPI2 OLED 驱动：

- `User/bsp_spi_oled.c/.h`：SPI2、SSD1306 初始化、清屏、文字测试和全屏图片刷新。
- `User/app_anim.c/.h`：非阻塞演示状态机。
- `User/anim_frames.c/.h`：两张 128x64 测试帧，常量数据位于 Flash。
- `Tools/gif_to_oled_frames.py`：在电脑端把 GIF 转成 SSD1306 页格式数组。

由于旧 I2C OLED 和新 SPI OLED 都需要独立刷新，本工程通过 `User/main.c` 中的
`SPI_OLED_ANIM_DEMO` 选择运行模式，不让两个显示模块同时抢占显示内容。

任务说明中的通用 `OLED_Init()`、`OLED_Clear()` 等名称已经被旧 I2C 驱动占用。
为保证两个驱动可以同时加入 Keil，新 SPI 驱动对应使用
`OLED_SPI_PanelInit()`、`OLED_SPI_Clear()`、`OLED_SPI_SetPos()` 和
`OLED_SPI_ShowImage128x64()`；`OLED_SPI_Init()` 专门负责 GPIO 与 SPI2 底层初始化。

## 2. 硬件接线

| OLED 引脚 | STM32F103 引脚 | 作用 |
| --- | --- | --- |
| SCK / D0 / SCL | PB13 | SPI2 时钟，由 STM32 输出 |
| MOSI / D1 / SDA | PB15 | SPI2 数据输出，发送命令和图像 |
| CS | PB12 | OLED 片选，低电平有效 |
| DC | PB14 | 低电平为命令，高电平为显示数据 |
| RST / RES | PB10 | OLED 硬件复位 |
| VCC | 3.3V | 模块电源，优先使用 3.3V |
| GND | GND | 电源地，必须与 STM32 共地 |

OLED 只接收数据，不需要 MISO。SPI2 使用主机、单向发送、8 位、MSB first、Mode 0、
软件 NSS，PB12 由程序直接控制 CS。SPI2 时钟来自 APB1，当前 8 分频约为 4.5 MHz。

## 3. 分阶段显示过程

上电后依次执行：

1. 初始化 SPI2，复位 SSD1306，发送初始化命令并清屏。
2. 非阻塞显示 `SPI OLED` 文字 1 秒。
3. 显示第一张 128x64 测试图 1 秒，确认页格式和方向。
4. 每 100 ms 在两张测试图之间切换。
5. 用转换脚本替换测试帧后，按相同任务循环播放多帧动画。

`App_Anim_Task()` 只比较 `Timing_GetTick()`，动画阶段没有 `delay_ms()`。
初始化阶段的 30 ms 延时仅用于 RST 硬件时序。

## 4. SSD1306 图片格式

128x64 单色全屏大小为：

```text
128 * 64 / 8 = 1024 字节
```

SSD1306 页格式共有 8 页，每页 128 字节。每个字节控制同一列中纵向连续的 8 个像素，
最低位对应该页最上方像素。`OLED_SPI_ShowImage128x64()` 按页写入全部 1024 字节。

## 5. SSD1306 与 SH1106

当前驱动按 SSD1306 128x64 编写。SH1106 常见模块内部列 RAM 为 132 列，显示区域通常
需要增加约 2 列偏移。如果 SH1106 出现整体左右偏移，应在 `OLED_SPI_SetPos()` 中给
列地址增加模块实际要求的偏移。不要同时修改 SSD1306 初始化和列偏移后再排错。

## 6. Keil 工程

`Project/led.uvprojx` 已加入以下源文件：

```text
User/bsp_spi_oled.c
User/app_anim.c
User/anim_frames.c
```

对应头文件位于 `User`，该目录已经在 Include Paths 中。若手工迁移到另一个工程，
应把三个 `.c` 文件加入 Keil Source Group，并确保包含 `User`、`SYSTEM/delay` 和
`SYSTEM/timing`。

SPI 动画模式需要：

```c
#include "bsp_spi_oled.h"
#include "app_anim.h"
```

初始化和主循环调用：

```c
delay_init();
Timing_Init();
OLED_SPI_PanelInit();
App_Anim_Init();

while (1)
{
    App_Anim_Task();
}
```

## 7. 两种运行模式

`User/main.c` 中：

```c
#define SPI_OLED_ANIM_DEMO 1U
```

- `1U`：单独动画演示模式，使用 SPI OLED。
- `0U`：恢复原有光敏传感器、交通灯和 I2C OLED 界面。

正常系统若还要运行其他传感器、UART 或状态机任务，可以继续在 `while (1)` 中调用，
但只能由一个明确的显示管理模块决定当前由 I2C UI 还是 SPI 动画刷新屏幕。

## 8. GIF 转换

### 最简单的使用方法

在工程根目录双击：

```text
一键更换GIF并烧录.bat
```

然后只需要选择一个 GIF。工具会自动完成：

```text
选择 GIF
→ 取前 20 帧并转换为 128x64 黑白数组
→ 覆盖 User/anim_frames.c 和 User/anim_frames.h
→ 调用 Keil 重新编译
→ 编译成功后调用 Keil 下载到开发板
```

默认参数为阈值 128、100 ms/帧、最多 20 帧。首次运行如果没有 Pillow，工具会询问
是否自动安装。烧录前应连接 ST-Link、给开发板供电，并在 Keil 的 Debug 和 Utilities
页面中选择实际使用的下载器。该下载器设置通常只需在第一次使用时配置。转换或编译
失败时不会继续烧录。

如果暂时没有连接开发板，烧录步骤会提示失败，但生成的数组和编译出的
`Output/led.hex` 仍然保留，之后连接 ST-Link 后可再次运行脚本，或在 Keil 中
点击 Download。

### 命令行使用方法

安装 Pillow：

```powershell
py -m pip install pillow
```

基本命令：

```powershell
py Tools/gif_to_oled_frames.py input.gif User/anim_frames.c
```

限制帧数、阈值、帧间隔并反色：

```powershell
py Tools/gif_to_oled_frames.py input.gif User/anim_frames.c --max-frames 20 --threshold 128 --interval 100 --invert
```

脚本会同时生成同目录、同文件名的 `.c` 和 `.h`。它执行以下步骤：

1. 读取 GIF 并拆分帧。
2. 等比例裁剪到 128x64。
3. 转灰度并按阈值二值化。
4. 可选黑白反色。
5. 按 SSD1306 页格式打包。
6. 输出 `const uint8_t anim_frames`，供固件直接从 Flash 读取。

先备份需要保留的手工测试帧，再用脚本覆盖 `User/anim_frames.c/.h`。重新打开 Keil 后
Rebuild 即可，不需要在 STM32 端读取 `.gif` 文件。

### 工程内置最终动画

工程已经提供可重复转换的实际 GIF：

```text
Assets/oled_demo.gif
```

该文件为 128x64、12 帧、100 ms/帧，画面包含移动方块、轨迹标记和进度条。
当前 `User/anim_frames.c/.h` 就是执行以下命令生成的最终固件数组：

```powershell
py Tools/gif_to_oled_frames.py Assets/oled_demo.gif User/anim_frames.c --max-frames 12 --threshold 128 --interval 100
```

生成结果为 12288 字节动画数据，保存在 Flash 中。转换脚本按工程文本规范输出
代码页 936、无 BOM 的 C/H 文件，可直接由 Keil ARMCC 5 编译。

## 9. Flash 与 RAM

| 帧数 | 仅帧数据约占 Flash |
| ---: | ---: |
| 1 | 1 KB |
| 10 | 10 KB |
| 20 | 20 KB |
| 30 | 30 KB |
| 60 | 60 KB |

STM32F103C8T6 标称通常为 64 KB Flash、20 KB RAM。建议先使用 8 至 20 帧、
5 至 12 FPS，默认 100 ms 一帧。数组声明为 `const`，不复制到 RAM；工程不使用
`malloc`，不动态加载图片，也不在 STM32 上解析 GIF。

## 10. 编译与烧录测试

1. 按接线表连接 OLED，确认供电为 3.3V 且共地。
2. 最简单方式是双击 `一键更换GIF并烧录.bat` 并选择 GIF。
3. 手工方式可打开 `Project/led.uvprojx`，执行 Rebuild 并确认 0 Errors。
4. 用 ST-Link 选择正确的 STM32F103C8 目标并下载。
5. 复位后观察“清屏、文字、单图、双帧切换”的顺序。
6. 若方向不对，只调整 `0xA0/0xA1` 和 `0xC0/0xC8`，每次只改一组后验证。

## 11. 排错清单

### 屏幕完全不亮

按顺序检查：

1. VCC 是否接 3.3V。
2. GND 是否共地。
3. RST 是否接 PB10。
4. CS 是否接 PB12。
5. DC 是否接 PB14。
6. SCK PB13 与 MOSI PB15 是否接反。
7. SPI2 和 GPIOB 时钟是否打开。
8. OLED 初始化命令是否发送。

### 屏幕亮但不显示

检查 DC 命令/数据电平是否反了、CS 是否有效、SPI Mode 是否为 Mode 0、SSD1306
初始化命令是否适配模块，以及模块是否实际为需要列偏移的 SH1106。

### 图像错位或只显示一部分

检查数组是否恰好 1024 字节、页地址和列地址是否正确、转换脚本是否按纵向 8 像素
打包，以及 SH1106 是否需要约 2 列偏移。

### 图像上下或左右反向

上下方向检查 COM scan direction 命令 `0xC0/0xC8`，左右方向检查 segment remap
命令 `0xA0/0xA1`。如果只有转换后的图片异常，再检查 Python 像素遍历和反色参数。

### 动画卡顿

检查动画任务中是否加入了 `delay_ms()`、主循环是否有其他阻塞任务、刷新间隔是否过短、
SPI 分频是否过慢，以及帧数组是否过多。当前全屏刷新约发送 1024 字节。

### 编译 Flash 超限

减少 GIF 帧数，使用 `--max-frames`，确认数组没有同时定义在多个文件，确认声明带
`const`，并删除不再使用的测试帧。60 帧已经接近 64 KB 芯片的全部标称 Flash。
