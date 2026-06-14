# ST7789 纯色循环验证

日期：2026-06-14

## 修改目标

把当前 ST7789 固件收敛为第一阶段纯色点屏验证，并为无 CS 七针模块提供
SPI Mode 0/Mode 3 编译期开关。

## 修改前行为

当前驱动已经使用 PB13/PB15 的 SPI2、PB10 复位、PB14 命令数据选择和
PB12 背光，也不依赖 CS。SPI 固定为 Mode 0，主循环每 500 ms 显示
黑、红、绿、蓝，缺少白色验收画面。

## 修改后行为

驱动默认使用 Mode 0，可把 `ST7789_SPI_MODE` 改为 `3U` 后重新编译以验证
Mode 3。主循环每 1000 ms 依次显示红、绿、蓝、白、黑，便于先确认初始化、
SPI 数据、颜色格式和可见区域，再继续图片或动画功能。

## 逻辑变化范围

- 新增 `ST7789_SPI_MODE`，只允许配置为 0 或 3。
- Mode 0 使用 CPOL Low、CPHA 1Edge；Mode 3 使用 CPOL High、CPHA 2Edge。
- 纯色保持时间由 500 ms 改为 1000 ms。
- 纯色顺序改为红、绿、蓝、白、黑。
- 保持 `ST7789_USE_CS=0`、GPIOB/SPI2 引脚、复位、背光和初始化序列不变。

## 涉及文件

- `User/bsp_st7789.h`
- `User/bsp_st7789.c`
- `User/main.c`
- `docs/change-logs/2026-06-14-st7789-solid-color-validation.md`

## 接口与兼容性

公开函数接口不变。默认行为仍使用 SPI Mode 0；只有无显示时才需要把
`ST7789_SPI_MODE` 改为 `3U` 并重新编译。接线保持 PB13=SCL、PB15=SDA、
PB10=RES、PB14=DC、PB12=BLK。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- STM32 文本策略检查：4 个目标文件通过。
- 新增函数注释检查：通过。
- `git diff --check`：通过。
- Mode 0 隔离 Keil 构建：0 Error，0 Warning。
- Mode 0 程序大小：Code=1976，RO-data=308，RW-data=24，ZI-data=1024。
- Mode 0 构建日志：`Output/codex-verify-20260614-185859-554/led.build_log.htm`。
- Mode 3 隔离 Keil 构建：0 Error，0 Warning。
- Mode 3 程序大小：Code=1980，RO-data=308，RW-data=24，ZI-data=1024。
- Mode 3 构建日志：`Output/codex-verify-20260614-185930-973/led.build_log.htm`。
- 最终源码已恢复默认 `ST7789_SPI_MODE=0U`。
- 硬件显示效果仍需烧录后确认。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：未创建；三个代码文件在本次会话开始前已有未提交修改，自动提交保护已拒绝混合提交。
- 提交说明：Add ST7789 solid color validation modes
