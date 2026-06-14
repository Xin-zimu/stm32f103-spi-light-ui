# ST7789 切换 GPIOA 并恢复 main.c 入口

日期：2026-06-12

## 修改目标

将 ST7789 最小诊断固件从 GPIOB 切换到一组新的 GPIOA 引脚，并恢复
`User/main.c` 作为 Keil 工程的实际入口，避免用户误烧录或误改未参与构建的入口文件。

## 修改前行为

软件 SPI 使用 PB13/PB15，RES 使用 PB10，DC 使用 PB14。Keil 工程入口为
`User/main_st7789.c`，而目录中保留的 `User/main.c` 不参与构建，容易造成误判。

## 修改后行为

软件 SPI 使用 PA5/PA7，RES 使用 PA3，DC 使用 PA4。Keil 工程直接编译
`User/main.c`，该文件只运行 ST7789 红、绿、蓝、白纯色循环和故障恢复。

## 逻辑变化范围

- 将 ST7789 GPIO 端口从 GPIOB 改为 GPIOA。
- 将 SCL、SDA、RES、DC 分别改为 PA5、PA7、PA3、PA4。
- 将 Keil 应用入口从 `main_st7789.c` 改为 `main.c`。
- 保持软件 SPI、完整 240x320 控制器显存填充和有界故障恢复逻辑不变。

## 涉及文件

- `Project/led.uvprojx`
- `User/main.c`
- `User/bsp_st7789.c`
- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-12-st7789-gpioa-main-entry.md`

## 接口与兼容性

公开的 `bsp_st7789.h` API 和颜色格式不变。硬件接线不兼容旧版本，必须按
PA5=SCL、PA7=SDA、PA3=RES、PA4=DC 重新接线。BLK 和 VCC 接 3.3V，GND 共地。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 文本规范检查：8 个涉及路径全部通过。
- 新增函数注释检查：通过。
- `git diff --check`：通过，无空白错误。
- Keil 隔离构建：0 Error(s)，0 Warning(s)。
- 构建日志确认实际编译 `main.c`，未编译 `main_st7789.c`。
- 程序大小：Code=1624，RO-data=312，RW-data=24，ZI-data=1024。
- 烧录文件：`Output/st7789_gpioa_soft_spi.hex`，5570 字节。
- SHA256：`B4474A0C53CB339CC078683682CC7197308C16B1D6F3344BFC628C8B6D01635F`。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：this commit
- 提交说明：Switch ST7789 diagnostic to GPIOA main entry
