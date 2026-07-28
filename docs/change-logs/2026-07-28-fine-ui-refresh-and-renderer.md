# UI 精细刷新和窄条带渲染

日期：2026-07-28

## 修改目标

一次性完成当前剩余计划：SETTINGS 更多设置项细粒度局部刷新、PLAYER 增加更多程序生成动画图形、INFO 页提供上板观察 DMA/dirty/UI 积压的数据，以及 renderer 从全宽 Y 条带升级为按 dirty `x/w` 的窄条带发送。

## 修改前行为

SETTINGS 修改动画开关和主题时仍刷新整行，只有亮度条使用较小 dirty。PLAYER 只有扫描条、移动块、波形和帧计数，动画压力场景较单一。INFO 页显示条带提交、renderer busy 和 dirty max/current，无法直接看到累计发送字节和 DMA busy。renderer 虽然消费 dirty 矩形，但每个 dirty 只按 Y 范围切条带，实际仍发送 240 像素全宽数据。

## 修改后行为

SETTINGS 动画开关只刷新右侧 toggle 区域，亮度只刷新进度条区域，主题只刷新右侧值区域；焦点移动仍刷新相关整行以保证选中底色和焦点标记一致。PLAYER 播放态新增背景短线、轨道点和拖尾块，继续由帧计数生成，不保存图片帧。INFO 页改为显示 STRIP、BYTES、BUSY、DMA、DIRTY，其中 DIRTY 为 max/current/overflow 低位。renderer 保留 dirty 的 `x/w`，每个 4 像素高条带只发送窄窗口；`ui_draw` buffer 写入改为相对条带坐标，保证窄窗口数据从 buffer 起始位置连续发送。

## 逻辑变化范围

- `ui_renderer` 的 strip.x/strip.w 改为来自 active dirty，统计新增累计发送字节。
- `ui_draw` 写 strip buffer 时使用 `(rect.x - strip.x)` 和 `strip.w` 计算行内偏移，支持窄条带。
- `page_settings` 新增动画 toggle 和主题值的控件级 dirty 矩形，值变更不再默认刷新整行。
- `page_player` 新增整数生成的背景短线、轨道点和拖尾块。
- `page_info` 显示累计发送字节、DMA busy 和 dirty overflow 低位，便于上板观察刷新压力。
- `README.md` 更新当前状态、构建尺寸、INFO 说明、窄条带状态和后续上板观察方法。

## 涉及文件

- `User/ui_draw.c`
- `User/ui_renderer.c`
- `User/ui_renderer.h`
- `User/page_settings.c`
- `User/page_player.c`
- `User/page_info.c`
- `README.md`
- `docs/change-logs/2026-07-28-fine-ui-refresh-and-renderer.md`

## 接口与兼容性

`UI_RendererStats` 新增 `bytes_submitted` 字段，现有字段和函数名不变。页面、按键、dirty 队列和 LCD DMA 对外调用方式不变。renderer 仍使用固定最大 240x4 双 buffer，不增加动态内存或全帧缓冲。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 中途 Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 中途程序尺寸：Code=13348, RO-data=1988, RW-data=40, ZI-data=6408。
- 中途构建日志：`Output\codex-verify-20260728-192819-571\led.build_log.htm`。
- `test-stm32-text-policy.ps1 -Paths User\ui_draw.c, User\ui_renderer.c, User\ui_renderer.h, User\page_settings.c, User\page_player.c, User\page_info.c, README.md, docs\change-logs\2026-07-28-fine-ui-refresh-and-renderer.md`：通过。
- `test-new-function-comments.ps1 -Paths User\ui_draw.c, User\ui_renderer.c, User\page_settings.c, User\page_player.c, User\page_info.c`：通过。
- `git diff --check`：通过。
- 最终 Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 最终程序尺寸：Code=13348, RO-data=1988, RW-data=40, ZI-data=6408。
- 最终构建日志：`Output\codex-verify-20260728-192932-552\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：61c8c3eddf4003b3b52b9b0da6ed8ad105c7412c
- Commit：this commit
- 提交说明：Refine UI refresh and narrow renderer strips
