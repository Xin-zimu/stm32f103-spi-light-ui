# ST7789 最小化安全驱动

日期：2026-06-12

## 修改目标

把工程收敛为只驱动 ST7789 的纯色诊断固件，移除当前构建中的动画和其他外设依赖，
并消除 SPI 与延时等待可能导致的永久卡死。

## 修改前行为

工程同时编译大量未使用的外设、旧 OLED、传感器、动画任务和动画数组。ST7789
驱动使用无上限 TXE、BSY 和延时等待，硬件状态异常时可能永久阻塞。上电直接播放
动画，不利于区分硬件、初始化和图片数据问题。

## 修改后行为

工程只编译 CMSIS、GPIO、RCC、SPI、ST7789 驱动、最小主程序和故障处理。初始化
成功后循环显示红、绿、蓝、白纯色。SPI 和延时均有超时，错误后重新初始化；严重
Cortex-M3 故障时请求系统复位。

## 逻辑变化范围

- ST7789 公开 API 改为返回 `ST7789_Status`。
- TXE、BSY 和周期计数器等待加入有限超时。
- SPI2 速度降为约 4.5 MHz，提高跳线连接可靠性。
- 主程序只运行四种纯色诊断和失败重试。
- 新增 Cortex-M3 严重故障复位处理。
- Keil 删除动画、传感器、旧 OLED、串口、定时器和无关外设库的编译项。
- GIF/Python 文件保留在磁盘，但不参与当前固件。

## 涉及文件

- `User/bsp_st7789.c`
- `User/bsp_st7789.h`
- `User/main_st7789.c`
- `User/fault_handlers.c`
- `Project/led.uvprojx`
- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-12-minimal-st7789-safe-driver.md`

## 接口与兼容性

`ST7789_Init()` 和 `ST7789_FillScreen()` 由 `void` 改为返回状态码。动画任务不再参与
当前构建。屏幕接线保持 PB13、PB15、PB10、PB14，BLK 直接接 3.3V。

## 编码与注释规范

- C/H 使用 ASCII，兼容代码页 936，无 BOM。
- Markdown 和 Keil 工程使用 UTF-8，无 BOM。
- 新增和修改函数均包含用途、参数、返回值、副作用和硬件约束说明。

## 验证结果

- 首次最小工程 Keil 隔离构建：0 错误，0 警告。
- 首次大小：Code=2044，RO-data=276，RW-data=24，ZI-data=1024。
- 文本编码检查：通过。
- 新增函数注释检查：通过。
- `git diff --check`：通过。
- 加入故障复位处理后的最终 Keil 隔离构建：0 错误，0 警告。
- 最终大小：Code=2088，RO-data=276，RW-data=24，ZI-data=1024。
- 已复制验证固件到 `Output/st7789_minimal.hex`，SHA256 为
  `C304AEF789E9253AB7B355CB3F15DB377075C57F7ABFA223156A585C2378D22D`。
- 原 `Output` 依赖文件被占用，常规 Rebuild 无法删除旧 `.d` 文件，因此没有覆盖
  旧 `Output/led.hex`。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：this commit
- 提交说明：Simplify ST7789 firmware and add bounded timeouts
