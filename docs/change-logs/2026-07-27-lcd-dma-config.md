# 抽离 LCD DMA 与统一配置

日期：2026-07-27

## 修改目标

执行计划阶段 A+B：新增统一配置入口，并把 SPI2 TX DMA 的状态、启动、错误恢复和 DMA1 Channel5 中断处理从 ST7789 驱动中抽离到独立 `lcd_dma` 模块。当前先保持 ST7789 同步绘制接口可用，为后续 UI 条带双缓冲渲染器提供独立 DMA 底座。

## 修改前行为

`bsp_st7789.c` 同时负责 ST7789 初始化、地址窗口、像素发送、DMA1 Channel5 初始化、DMA busy/error 状态、DMA 启动、错误恢复和中断入口。UI 参数、按键参数和动画参数分散在多个头文件中。

## 修改后行为

新增 `app_config.h` 集中保存屏幕、DMA、UI、按键、动画、GIF 和调试开关参数。现有 `key_driver`、`ui_types`、`ui_event`、`ui_dirty`、`ui_anim`、`ui_feedback` 的公开宏继续保留，但改为引用统一配置项。

新增 `lcd_dma.c/.h`，由该模块统一管理 SPI2 TX DMA1 Channel5：

- `LCD_DMA_Init()` 配置 DMA 通道、中断和 SPI2 TX DMA 请求。
- `LCD_DMA_Start()` 只在 IDLE 状态接受一次传输。
- `LCD_DMA_Task()` 在主循环语境释放完成状态或恢复错误状态。
- `DMA1_Channel5_IRQHandler()` 迁移到 `lcd_dma.c`，只记录完成/错误并清理通道。

`bsp_st7789.c` 保留现有绘制 API，通过兼容包装调用 `lcd_dma`，所以当前 UI 行为保持不变。

## 逻辑变化范围

- 新增统一配置头文件，集中原本分散的 UI、按键、DMA 和后续 GIF 参数。
- 新增 LCD DMA 状态机和传输描述符，迁移 DMA 初始化、启动、等待、错误恢复和 DMA1 Channel5 IRQ。
- `ST7789_SPI_Init()` 不再直接初始化 DMA，而是调用 `LCD_DMA_Init()`。
- ST7789 现有同步绘制和旧 GIF 差分辅助路径仍保留，通过 `ST7789_TryStartBufferDMA()` 包装到 `LCD_DMA_Start()`。
- 本阶段没有接入 UI 条带渲染器，也没有恢复 GIF 构建。

## 涉及文件

- `User/app_config.h`
- `User/lcd_dma.c`
- `User/lcd_dma.h`
- `User/bsp_st7789.c`
- `User/key_driver.h`
- `User/ui_anim.h`
- `User/ui_dirty.h`
- `User/ui_event.h`
- `User/ui_feedback.h`
- `User/ui_types.h`
- `Project/led.uvprojx`
- `docs/change-logs/2026-07-27-lcd-dma-config.md`

## 接口与兼容性

对应用层入口不变：`Key_Task(now)`、`App_UI_Task(now)` 和页面模块不需要修改。ST7789 公开绘制接口不变，当前仍是 UI-first 构建，GIF 数据和播放模块仍未加入 Keil 构建。

新增内部接口 `LCD_DMA_*`，供后续 `ui_renderer` 和 GIF 控件复用。DMA 中断入口现在由 `lcd_dma.c` 提供，避免 ST7789 驱动继续持有 DMA 状态。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，12 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- Keil isolated build：通过。
  - 工程：`Project\led.uvprojx`
  - 输出：`Output\codex-verify-20260727-165359-035`
  - 结果：0 error(s), 0 warning(s)
  - 大小：`Code=10768 RO-data=1984 RW-data=16 ZI-data=2432`

## Git

- 分支：master
- 起始提交：38d60d82b151129b0d10824e54fdb754cb9206c4
- Commit：this commit
- 提交说明：Extract LCD DMA and centralize config
