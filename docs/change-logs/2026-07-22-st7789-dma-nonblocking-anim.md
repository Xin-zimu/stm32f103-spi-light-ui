# ST7789 DMA 优化与非阻塞动画

日期：2026-07-22

## 修改目标

按讨论的 2、3、6 三项继续优化 ST7789 SPI DMA：

- 将 GIF 差分动画刷新改为 DMA 中断驱动的非阻塞状态机。
- 优化纯色清屏/矩形填充，避免每一行重复生成相同颜色数据。
- 增加 DMA 传输错误检测与恢复，避免异常后显示链路卡死。

## 修改前行为

- `ST7789_ApplyIndexed4Delta()` 会在一次函数调用内解析并发送完整差分帧；底层 DMA 已由中断释放忙标志，但同步接口仍会等待每段传输完成后继续推进。
- `ST7789_Clear()` 和 `ST7789_FillRect()` 每发送一行前都会重新填充行缓冲区，即使整块区域颜色完全相同。
- DMA 中断能够释放完成状态，但遇到传输错误时只记录错误标志，缺少面向下一次传输的恢复路径。

## 修改后行为

- 新增 `ST7789_AnimDeltaStart()`、`ST7789_AnimDeltaTask()`、`ST7789_AnimDeltaBusy()`，动画差分刷新由主循环分步推进；DMA 忙时任务立即返回，CPU 可继续执行其他主循环任务。
- `App_ST7789_AnimTask()` 使用新的异步差分接口，只有一个完整 transition 完成后才推进帧序号和下一帧 deadline。
- 纯色清屏和矩形填充预先构建两个相同行缓冲区，后续只交替发 DMA，减少重复 CPU 填充。
- DMA TE 中断后会关闭 DMA 通道、清除 DMA1 Channel5 挂起位、重新打开 SPI2 TX DMA 请求，并清理软件忙/错状态。

## 逻辑变化范围

- `User\bsp_st7789.c`：新增异步差分刷新 job 状态、DMA ready/try-start/recover 辅助函数，并保留原同步显示接口。
- `User\bsp_st7789.c`：调整 `ST7789_Clear()`、`ST7789_FillRect()` 的纯色行缓冲生成策略。
- `User\app_st7789_anim.c`：动画任务接入异步差分刷新状态机。
- `User\bsp_st7789.h`：公开异步差分刷新接口。

## 涉及文件

- `User\bsp_st7789.c`
- `User\bsp_st7789.h`
- `User\app_st7789_anim.c`
- `docs\change-logs\2026-07-22-st7789-dma-nonblocking-anim.md`

## 接口与兼容性

- 新增接口：
  - `ST7789_AnimDeltaStart(...)`
  - `ST7789_AnimDeltaTask(void)`
  - `ST7789_AnimDeltaBusy(void)`
- 原有 `ST7789_ShowIndexed4Image()`、`ST7789_ApplyIndexed4Delta()`、`ST7789_Clear()`、`ST7789_FillRect()` 等接口保持可用。
- 硬件映射仍为 SPI2 TX DMA1 Channel5，未改变引脚、电平和现有屏幕初始化流程。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1`：4 个文件通过。
- `test-new-function-comments.ps1`：通过。
- Keil isolated build：`0 error(s), 0 warning(s)`。
- 程序体积：`Code=8106 RO-data=56746 RW-data=4 ZI-data=2268`。

## Git

- 分支：codex/usart1-tx-dma
- 起始提交：82d5a1a8a9f660f2cdb2a5da81179861513b32be
- Commit：this commit
- 提交说明：Optimize ST7789 DMA animation pipeline
