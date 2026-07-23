# ST7789 DMA 文档和差分校验

日期：2026-07-23

## 修改目标

同步 ST7789 SPI2 TX DMA 现状文档，并收紧异步 GIF 差分流运行时校验。
文档需要准确描述当前“命令同步 SPI、像素流 DMA、差分任务异步推进”的实现，
代码需要在差分数据损坏时更早进入整帧恢复路径。

## 修改前行为

README 仍写 SPI2 约 4.5 MHz，未说明像素流已经走 DMA，也没有列出
`stm32f10x_dma.c`。`docs/spi_oled_gif_animation.md` 仍写“不是 DMA 后台刷屏”，
与当前驱动实现不一致。

异步差分任务在扫描源图尚未完成时，如果 `position >= delta_size`，会把该状态
当作正常完成；完整源图扫描结束后也没有拒绝多余尾部字节。

## 修改后行为

README 和 GIF 动画说明已改为当前实现：

- ST7789 初始化、命令、控制参数和地址窗口仍使用同步 SPI 字节发送。
- 清屏、矩形填充、完整帧和 GIF 差分像素流使用 SPI2 TX DMA。
- SPI2 TX 使用 DMA1 Channel5，DMA 完成中断释放发送状态。
- 差分动画由裸机协作式异步状态机推进，不创建全屏帧缓冲。
- SPI2 时钟描述改为约 18 MHz。
- 工程文件列表补充 USART 和 DMA 标准外设库源文件。

异步差分任务保留 `delta_size == 0` 的无变化帧语义；非零差分流必须完整覆盖
源图且不能携带多余尾部字节。提前结束或尾部多余数据都会返回
`ST7789_ANIM_DELTA_RESULT_ERROR`，由上层重画完整首帧恢复。

## 逻辑变化范围

- `ST7789_AnimDeltaStart()`：`delta_size == 0` 时直接接受为空更新并保持任务空闲。
- `ST7789_AnimDeltaTask()`：源图尚未扫描完成但差分流耗尽时返回错误。
- `ST7789_AnimDeltaTask()`：源图扫描完成后若 `position != delta_size`，返回错误。
- 文档和 `main.c` 注释同步 DMA 行为，不改变引脚和外部绘图接口。

## 涉及文件

- `README.md`
- `User/bsp_st7789.c`
- `User/main.c`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-07-23-st7789-dma-docs-delta-check.md`

## 接口与兼容性

无外部 API 破坏。`ST7789_AnimDeltaStart()` 对空差分流仍返回接受；
非零损坏差分流会更早返回错误并触发既有整帧恢复逻辑。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，检查 5 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过；仅出现 Git 换行提示，无空白错误。
- Keil 隔离构建：通过，0 Error(s)、0 Warning(s)。
  程序大小：Code=8266，RO-data=56746，RW-data=4，ZI-data=2268。

## Git

- 分支：master
- 起始提交：af8925bf879104980ac0f614d2b98be8b556d467
- Commit：this commit
- 提交说明：Sync ST7789 DMA docs and validate delta bounds
