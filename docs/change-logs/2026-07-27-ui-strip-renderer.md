# UI 条带双缓冲渲染器

日期：2026-07-27

## 修改目标

执行计划阶段 C/D：新增 UI 条带双缓冲渲染器，把页面绘制从 `UI_PageTask()` 直接调用 `ST7789_FillRect()` 的同步链路，改为先绘制到固定高度条带 buffer，再通过 `lcd_dma` 异步发送到 ST7789。

## 修改前行为

`UI_PageTask()` 直接从 `ui_dirty` 弹出脏矩形，设置 `ui_draw` 裁剪区，清裁剪区背景，然后调用当前页面 `draw()`。`ui_draw` 内部的矩形和文字最终直接调用 `ST7789_FillRect()`，每个矩形绘制都可能等待 DMA 完成，UI 页面层仍与硬件写屏强耦合。

## 修改后行为

新增 `ui_renderer.c/.h`，渲染器负责消费 `ui_dirty`，把 dirty 的 Y 范围切成 `APP_UI_STRIP_HEIGHT=4` 的水平条带。每个条带使用两个静态 RGB565 buffer 之一绘制，绘制完成后设置 ST7789 地址窗口并调用 `LCD_DMA_Start()` 发送。

`ui_draw` 增加 buffer 后端：

- `UI_DrawBeginBuffer()` / `UI_DrawEndBuffer()` 切换到条带 buffer 绘制。
- `UI_DrawRect()` 在 buffer 上下文中写 RGB565 高字节在前的像素数据。
- 文字、菜单行、状态栏、页脚、进度条和焦点条继续调用原有 `ui_draw` API，页面模块无需改动。

`UI_PageTask()` 现在只推进反馈、页面动画和 `UI_RendererTask()`。脏区消费、背景清理和页面 draw 调用都转移到渲染器内部。

本阶段第一版为可靠性优先：每个 dirty 条带发送整屏宽 240 像素，只按 Y 范围减少刷新量。后续可在 renderer 中增加窄条带打包，进一步减少 X 方向带宽。

## 逻辑变化范围

- 新增 `ui_renderer` 状态机和两个 240x4 RGB565 条带缓冲。
- `ui_draw` 从纯 ST7789 直写改为支持 renderer-owned buffer 后端；未进入 buffer 上下文时保留直接写屏兼容路径。
- `UI_PageInit()` 初始化 renderer。
- `UI_PageTask()` 不再直接调用 `UI_DirtyPop()` 和 `UI_DrawClearClip()`，改为调用 `UI_RendererTask(now, current_page_ops)`。
- `bsp_st7789` 新增 `ST7789_BeginDataWrite()` 和 `ST7789_WaitWriteComplete()`，供 renderer 正确处理 DC 数据模式和 SPI BSY 时序。
- 当前仍未恢复 GIF 构建，也未实现窄 X 范围条带打包。

## 涉及文件

- `User/ui_renderer.c`
- `User/ui_renderer.h`
- `User/ui_draw.c`
- `User/ui_draw.h`
- `User/ui_page.c`
- `User/bsp_st7789.c`
- `User/bsp_st7789.h`
- `Project/led.uvprojx`
- `docs/change-logs/2026-07-27-ui-strip-renderer.md`

## 接口与兼容性

页面模块接口保持不变，`PAGE_*_OPS.draw(const UI_Rect *clip)` 仍按原方式绘制页面元素。应用入口 `App_UI_Init()` / `App_UI_Task(now)` 和按键模块不变。

新增内部接口：

- `UI_RendererInit()`
- `UI_RendererTask(now, page)`
- `UI_RendererIsBusy()`
- `UI_RendererRequestFull()`
- `UI_DrawBeginBuffer()`
- `UI_DrawEndBuffer()`
- `ST7789_BeginDataWrite()`
- `ST7789_WaitWriteComplete()`

RAM 占用增加约 3840 字节条带 buffer，加少量 renderer 状态变量。不使用完整 240x240 framebuffer，不使用动态内存。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，9 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- Keil isolated build：通过。
  - 工程：`Project\led.uvprojx`
  - 输出：`Output\codex-verify-20260727-171354-015`
  - 结果：0 error(s), 0 warning(s)
  - 大小：`Code=11196 RO-data=1984 RW-data=40 ZI-data=6336`
- 残留检查：UI 页面任务不再直接消费 `UI_DirtyPop()`；DMA 发送由 `ui_renderer` 调用 `LCD_DMA_Start()`。

## Git

- 分支：master
- 起始提交：c4d4bde923571c94c9d6a82be7cd3395cf079156
- Commit：this commit
- 提交说明：Add UI strip renderer
