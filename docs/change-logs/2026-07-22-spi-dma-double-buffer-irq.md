# SPI DMA 双缓冲与中断完成

日期：2026-07-22

## 修改目标

把 ST7789 的 SPI2 TX DMA 从单缓冲同步等待升级为双行缓冲，并使用 DMA1 Channel5 中断完成通知，减少 CPU 等待 DMA 标志的时间。

## 修改前行为

ST7789 驱动只有一个 480 字节行缓冲。每次构建一行或一段 RGB565 数据后，启动 DMA1 Channel5，并在发送函数里轮询 `DMA_GetFlagStatus(DMA1_FLAG_TC5)`，等待 DMA 完成后再继续构建下一行。

## 修改后行为

ST7789 驱动使用两个 480 字节行缓冲。当前缓冲启动 DMA 发送后，CPU 可以准备另一个缓冲；下一次启动 DMA 前只等待 `DMA1_Channel5_IRQHandler` 释放忙标志。DMA 完成由 TC/TE 中断处理，最终切换 DC 或地址窗口前仍等待 SPI2 `BSY` 清零，保证显示时序不被破坏。

## 逻辑变化范围

- SPI2 TX DMA 缓冲区从 1 个扩展为 2 个行缓冲。
- `ST7789_SPI_Init` 增加 DMA1 Channel5 TC/TE 中断配置和 NVIC 配置。
- 新增 `ST7789_WaitDMAReady`、`ST7789_StartBufferDMA`、`ST7789_FinishBufferDMA`，把 DMA 启动和收尾分离。
- 新增 `DMA1_Channel5_IRQHandler`，由中断关闭 DMA 通道、清中断挂起位并释放忙标志。
- 清屏、矩形填充、首帧显示和差分段显示改为交替使用双缓冲。
- 差分数据异常提前退出时，先收尾当前 DMA，避免返回后仍在发送。

## 涉及文件

- `User\bsp_st7789.c`
- `docs\change-logs\2026-07-22-spi-dma-double-buffer-irq.md`

## 接口与兼容性

对外接口不变，上层动画播放代码不需要修改。SPI2 TX 仍使用 STM32F103 固定映射的 DMA1 Channel5；USART1 TX DMA 仍使用 DMA1 Channel4，二者不冲突。DMA 对上层仍表现为同步刷屏：显示函数返回前会等待最后一次 DMA 和 SPI2 移位完成。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。
- 已验证触碰文件符合编码策略；新增/修改函数已补充函数前说明。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1`：通过，2 个路径。
- `test-new-function-comments.ps1`：通过。
- Keil 隔离构建：通过，0 Error(s)，0 Warning(s)。大小：Code=6010，RO-data=56746，RW-data=4，ZI-data=2212。

## Git

- 分支：codex/usart1-tx-dma
- 起始提交：e62c79e1707144b42277acaae77e4f0325000046
- Commit：this commit
- 提交说明：Add SPI DMA double buffering and IRQ completion
