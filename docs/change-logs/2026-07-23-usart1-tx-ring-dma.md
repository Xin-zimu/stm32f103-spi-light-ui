# USART1 TX 环形缓冲 DMA

日期：2026-07-23

## 修改目标

完善 USART1 TX DMA，把原来的单次 DMA 发送升级为内部环形缓冲加 DMA
自动续传。保持 USART1 RX 中断接收不变，避免 USART1_RX DMA 与 ST7789
SPI2_TX 同用 DMA1 Channel5 产生通道冲突。

## 修改前行为

`USART1_DMA_Send()` 直接使用调用者传入的缓冲区启动 DMA1 Channel4。
如果上一次 DMA 仍在发送，函数立即返回失败。调用者必须在 DMA 完成前保持
源缓冲区不变，`printf` 的 `fputc()` 也会等待 DMA 与 USART 完全空闲后再
同步写一个字节。

## 修改后行为

USART1 TX 增加 512 字节内部环形缓冲，保留一个空槽区分满和空。
`USART1_DMA_Send()` 现在先把整段数据复制进环形缓冲，只有空间不足时才返回
失败；如果 DMA 空闲，会立即启动 DMA1 Channel4。DMA 完成中断推进 tail，
并自动启动下一段连续内存，处理环形缓冲回绕。

`fputc()` 改为把单字节写入环形缓冲；如果缓冲满，会等待 DMA 中断释放空间，
保持 `printf` 的阻塞输出语义。新增 `USART1_DMA_Free()` 用于查询当前 TX
环形缓冲剩余空间。

## 逻辑变化范围

- 新增 `USART1_TX_BUFFER`、head、tail、当前 DMA 段长度和 busy 状态。
- 新增环形缓冲索引推进、已用空间、空闲空间和下一段 DMA 启动内部函数。
- `USART1_DMA_Send()` 改为复制到 TX 环形缓冲，并在临界区内尝试启动 DMA。
- `USART1_DMA_Free()` 暴露剩余可排队字节数。
- `USART1_DMA_IsBusy()` 同时检查环形缓冲、DMA 段和 USART 最后一位发送状态。
- `DMA1_Channel4_IRQHandler()` 完成当前段后推进 tail，并自动续传下一段。
- `fputc()` 改为排队单字节，不再直接写 USART1->DR。

## 涉及文件

- `SYSTEM/usart/usart.c`
- `SYSTEM/usart/usart.h`
- `docs/change-logs/2026-07-23-usart1-tx-ring-dma.md`

## 接口与兼容性

保留 `USART1_DMA_Send()` 和 `USART1_DMA_IsBusy()` 名称，但
`USART1_DMA_Send()` 的语义从“启动一次调用者缓冲区 DMA”变为“复制并排队”。
调用者现在可以在函数返回后立即复用源缓冲区。新增
`USART1_DMA_Free()`。USART1 RX 仍使用原中断接收状态机，不使用 RX DMA。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，检查 3 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过。
- Keil 隔离构建：通过，0 Error(s)、0 Warning(s)。
  程序大小：Code=8444，RO-data=56748，RW-data=4，ZI-data=2788。

## Git

- 分支：master
- 起始提交：55b94b554522c4e7ec967944cbb0c3d27b8f0ee1
- Commit：this commit
- 提交说明：Add USART1 TX ring buffer DMA queue
