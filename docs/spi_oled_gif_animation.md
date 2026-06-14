# ST7789 最小诊断固件说明

## 目标

当前阶段只验证 ST7789 240x240 屏幕是否能够可靠初始化和接收 RGB565 数据。
GIF、动画压缩和其他外设均从 Keil 构建中移除，避免干扰硬件排查。

## 构建组成

Keil 当前编译：

- CMSIS 启动和系统时钟。
- 标准外设库 GPIO、RCC、SPI。
- `User/main.c`。
- `User/bsp_st7789.c`。
- `User/fault_handlers.c`。

工程不编译：

- SSD1306 和旧 I2C OLED。
- 动画任务和动画数组。
- Python 生成的数据。
- 光敏传感器、交通灯、串口和 TIM3。
- 动画定时和其他通用任务模块。

## 当前通信配置

```text
SPI2 SCK  -> PB13
SPI2 MOSI -> PB15
RES       -> PB10
DC        -> PB14
BLK       -> PB12
SPI Mode  -> Mode 3
频率      -> 约 4.5 MHz
```

该模块没有 MISO 和外部 CS，STM32 只向屏幕发送数据。

## 两图切换画面

初始化成功后每秒切换两张测试图片。源图片为120x120、16色4位索引格式，
每张占7200字节，发送时实时放大2倍覆盖X=0至239、Y=0至239。
图片和调色板保存在Flash，不需要RAM帧缓冲。

观察结果：

- 暖色数字1与冷色数字2稳定交替：图片数据和循环逻辑正确。
- 两张图片均完整覆盖：索引解码、放大和地址窗口正确。
- 切换后没有残留：每帧发送了完整240x240像素。
- 有画面但红蓝颠倒：调整 MADCTL 的 BGR 位。
- 只有部分画面：检查模块分辨率、地址范围和 X/Y 偏移。
- 背光亮但始终无画面：重点检查烧录、RES、DC、SCL、SDA 和驱动芯片型号。

## 接线

```text
GND -> GND
VCC -> 3.3V
SCL -> PB13
SDA -> PB15
RES -> PB10
DC  -> PB14
BLK -> PB12
```

该模块没有 CS 引脚，驱动假设 CS 已在模块内部固定为有效电平。

详细的 PulseView 使用方法、SPI 解码和故障复盘见
[ST7789 与逻辑分析仪排障学习指南](st7789-logic-analyzer-troubleshooting-guide.md)。
