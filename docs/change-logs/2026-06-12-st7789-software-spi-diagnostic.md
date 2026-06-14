# ST7789 软件 SPI 诊断

日期：2026-06-12

## 修改目标

针对最小硬件 SPI 固件烧录后仍只有背光的问题，进一步排除 SPI2 配置、状态标志、
通信频率和 0/80 行偏移因素。

## 修改前行为

驱动通过 SPI2 向 240x240 地址窗口发送纯色，仍依赖 SPI2 配置和模块纵向偏移设置。

## 修改后行为

PB13 和 PB15 改为普通 GPIO 软件 SPI，按 Mode 0、MSB first 固定发送。初始化恢复
完整的 ST7789 porch、电源、帧率和 gamma 设置。纯色填充覆盖完整 240x320 控制器
显存，因此常见的可视区域 0 行或 80 行偏移都应显示颜色。

## 逻辑变化范围

- 移除 SPI2 外设初始化、TXE/BSY 轮询和 SPI 库编译项。
- PB13、PB15 改为推挽 GPIO 软件 SPI。
- 软件 SPI 每字节固定发送 8 位，所有循环均有固定结束条件。
- 恢复常见 ST7789 完整初始化寄存器序列。
- 纯色填充由 240x240 地址窗口改为完整 240x320 GRAM。
- 生成独立烧录文件 `Output/st7789_soft_spi.hex`。

## 涉及文件

- `User/bsp_st7789.c`
- `User/bsp_st7789.h`
- `User/main_st7789.c`
- `Project/led.uvprojx`
- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-12-st7789-software-spi-diagnostic.md`

## 接口与兼容性

`ST7789_FillScreen()` 更名为 `ST7789_FillControllerRam()`。引脚接线不变：
PB13=SCL、PB15=SDA、PB10=RES、PB14=DC。该驱动仍假设无 CS 模块内部已固定片选。

## 编码与注释规范

- C/H 使用 ASCII，兼容代码页 936，无 BOM。
- Markdown 和 Keil 工程使用 UTF-8，无 BOM。
- 新增和修改函数均包含完整函数头注释。

## 验证结果

- 文本编码检查：通过。
- 新增函数注释检查：通过。
- `git diff --check`：通过。
- Keil 隔离构建：0 错误，0 警告。
- 固件大小：Code=1648，RO-data=312，RW-data=24，ZI-data=1024。
- `Output/st7789_soft_spi.hex` 大小为 5631 字节，SHA256 为
  `A9D05E666F32806A984FF5061EADD35D33681E786E106A07EE006CFF742AFBF8`。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：this commit
- 提交说明：Add ST7789 software SPI diagnostic
