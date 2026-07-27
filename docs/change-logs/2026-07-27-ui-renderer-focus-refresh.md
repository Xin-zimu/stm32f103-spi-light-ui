# UI 条带渲染焦点刷新修复

日期：2026-07-27

## 修改目标

修复条带渲染版本中菜单焦点快速移动时的刷新残留问题。重点降低旧选中行清理被动画脏区挤压的概率，并让 renderer 在 DMA 空闲时更快启动下一条带传输。

## 修改前行为

`UI_FocusAnimBuildDirty()` 仍按旧的直接写屏模型生成 5 像素宽的蓝条脏区，并通过 `UI_DirtyAddIsolated()` 独立入队。条带 renderer 会把任何脏区转换成全宽条带发送，导致窄蓝条脏区并不再便宜，且无法和旧/新选中行清理合并。快速连续下移时，旧行黄色选中背景容易只刷新一部分，屏幕上留下短黄色横条，蓝色焦点条也会滞后。

`UI_RendererTask()` 每次调用只推进一个状态，完整提交一个 4 像素条带需要多次主循环调度，进一步放大快速按键时的刷新积压。

## 修改后行为

焦点动画脏区改为覆盖旧/新焦点所在的完整行级 Y 范围，并允许通过 `UI_DirtyAdd()` 与旧/新行重绘合并。这样选中背景、行内容和移动中的蓝色焦点条会作为同一行带刷新，减少半行残留。

`UI_RendererTask()` 改为在单次调用内连续推进 CPU-only 状态，直到提交一个 DMA 条带、DMA 忙、无脏区可刷或达到步数上限。主循环仍保持协作式返回，但不会再因为“每次只走一步”拖慢条带提交。

## 逻辑变化范围

- `UI_FocusAnimBuildDirty()` 从窄蓝条区域改为完整行级区域。
- 首页和设置页的焦点动画 dirty 从 isolated 入队改为普通可合并入队。
- `UI_RendererTask()` 增加有限步数循环，压缩 IDLE/PREPARE/DRAW/SUBMIT 等状态之间的空转调度。
- 未改变页面绘制接口、LCD DMA 接口、ST7789 接口和条带 buffer 尺寸。

## 涉及文件

- `User/ui_anim.c`
- `User/page_home.c`
- `User/page_settings.c`
- `User/ui_renderer.c`

## 接口与兼容性

无公开接口变化。现有页面仍通过 `UI_PageInvalidate()` / `UI_DirtyAdd()` 请求刷新，renderer 仍使用双条带 buffer 和 LCD DMA 输出。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1 -Paths User\ui_anim.c, User\ui_renderer.c, User\page_home.c, User\page_settings.c`：通过。
- `test-new-function-comments.ps1 -Paths User\ui_anim.c, User\ui_renderer.c, User\page_home.c, User\page_settings.c`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=11000, RO-data=1984, RW-data=40, ZI-data=6336。
- 构建日志：`Output\codex-verify-20260727-173506-593\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：da8fd2a4141e4cecda4e3fe35bfbf18316fb3e22
- Commit：this commit
- 提交说明：Fix UI focus strip refresh backlog
