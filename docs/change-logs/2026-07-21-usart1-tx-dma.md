# USART1 TX DMA

日期：2026-07-21

## 修改目标

在项目中加入 USART1 TX DMA 支持，使固件启动时初始化 USART1，并提供基于 DMA1 Channel4 的非阻塞发送接口。

## 修改前行为

Keil 工程未编入 `SYSTEM\usart`、`stm32f10x_usart.c` 和 `stm32f10x_dma.c`。主程序未初始化 USART1，串口发送只有旧的阻塞式 `fputc` 代码，且该文件加入 ARM Compiler 6 构建后会暴露旧 ARMCC5 专用 retarget 代码不兼容问题。

## 修改后行为

工程编入 USART 和 DMA 标准库源文件以及 `SYSTEM\usart` 模块。`main` 启动时以 115200 波特率初始化 USART1。`USART1_DMA_Send` 可以发起 USART1 TX DMA 非阻塞发送，`USART1_DMA_IsBusy` 可查询 DMA 和 USART 发送线是否空闲，`DMA1_Channel4_IRQHandler` 在 DMA 完成或错误后释放发送忙状态。

## 逻辑变化范围

- USART1 初始化增加 DMA1 时钟、DMA1 Channel4 配置、DMA 中断配置和 USART DMA TX 请求使能。
- 新增 USART1 TX DMA 发送接口和忙状态查询接口。
- 新增 DMA1 Channel4 中断处理，完成或错误时关闭 DMA 通道并清除状态。
- `fputc` 保留阻塞发送兼容路径，并在 DMA 正忙时等待，避免阻塞发送和 DMA 发送数据交错。
- ARMCC5 专用半主机 retarget 代码仅在 `__CC_ARM` 下启用，使当前 ARM Compiler 6 构建可通过。
- `main` 增加 `uart_init(115200)`。

## 涉及文件

- `Project\led.uvprojx`
- `SYSTEM\usart\usart.c`
- `SYSTEM\usart\usart.h`
- `User\main.c`
- `docs\change-logs\2026-07-21-usart1-tx-dma.md`

## 接口与兼容性

新增接口：

- `u8 USART1_DMA_Send(const u8 *data, u16 len)`
- `u8 USART1_DMA_IsBusy(void)`

保留原有 `uart_init(u32 bound)` 和 `fputc`。USART1 使用 PA9/PA10，TX DMA 使用 STM32F103 固定映射的 DMA1 Channel4。调用 DMA 发送后，发送缓冲区必须保持有效且不被修改，直到 `USART1_DMA_IsBusy()` 返回 0。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。
- 已验证触碰文件符合编码策略；新增/修改函数已补充函数前说明。

## 验证结果

- `git diff --check`：通过；Git 对 `Project\led.uvprojx` 和 `User\main.c` 输出 CRLF 信息提示，文件检测仍为 CRLF。
- `test-stm32-text-policy.ps1`：通过，5 个路径。
- `test-new-function-comments.ps1`：通过。
- Keil 隔离构建：通过，0 Error(s)，0 Warning(s)。大小：Code=6278，RO-data=56746，RW-data=4，ZI-data=1252。

## Git

- 分支：codex/usart1-tx-dma
- 起始提交：ab47845be0f9a183d81a516ce421f1517115b92a
- Commit：this commit
- 提交说明：Add USART1 TX DMA
