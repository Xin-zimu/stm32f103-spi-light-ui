# UI renderer 阶段 C/D 收尾

日期：2026-07-27

## 修改目标

完成阶段 C/D 收尾：把 UI 条带双缓冲 renderer 的当前状态写入 README，并补充 renderer / dirty 轻量统计接口，为下一阶段 GIF 与 UI 共用 LCD DMA 调度提供可观察数据。

## 修改前行为

README 仍保留上一阶段描述，提到动画窄脏区和旧构建尺寸，未说明当前 `ui_renderer` 已经接管 dirty 消费、条带 buffer 绘制和 LCD DMA 提交。代码中也没有统一的 renderer/dirty 统计读取接口，后续接入 GIF 时难以判断是否发生 DMA 忙、dirty 溢出或 UI 刷新积压。

## 修改后行为

README 更新为当前阶段 C/D 状态：条带双缓冲已可用，`ui_draw` 已写入 strip buffer，快速上下移动残留和蓝条断裂已修复，当前限制是仍按 dirty Y 范围发送全宽条带。

`ui_dirty` 增加统计结构和读取/重置接口，可观察 dirty 溢出次数、全屏升级次数、最大 pending 数和当前 pending 状态。`ui_renderer` 增加统计结构和读取/重置接口，可观察 task 调用、忙返回、DMA 忙、无空 buffer、submit busy、dirty 消费和条带绘制/提交次数。

## 逻辑变化范围

- 新增 `UI_DirtyStats`、`UI_DirtyGetStats()`、`UI_DirtyResetStats()`。
- 新增 `UI_RendererStats`、`UI_RendererGetStats()`、`UI_RendererResetStats()`。
- dirty 溢出和全屏刷新路径记录计数；dirty 入队记录最大 pending 数。
- renderer 任务、忙返回、条带绘制和 DMA 提交路径记录计数。
- README 更新当前构建尺寸、刷新架构、阶段 C/D 验收状态和下一阶段 GIF 调度计划。

## 涉及文件

- `User/ui_dirty.c`
- `User/ui_dirty.h`
- `User/ui_renderer.c`
- `User/ui_renderer.h`
- `README.md`

## 接口与兼容性

新增调试/调度观察接口，不改变现有绘制、dirty 入队、页面和 LCD DMA 调用方式。接口只复制内部计数器，不暴露 dirty 队列或 renderer 状态机内部结构。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1 -Paths User\ui_dirty.c, User\ui_dirty.h, User\ui_renderer.c, User\ui_renderer.h, README.md`：通过。
- `test-new-function-comments.ps1 -Paths User\ui_dirty.c, User\ui_dirty.h, User\ui_renderer.c, User\ui_renderer.h`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=11288, RO-data=1984, RW-data=40, ZI-data=6376。
- 构建日志：`Output\codex-verify-20260727-183031-939\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：7a7001ac985a94108bcf4a0c01d6fbdd2c278614
- Commit：this commit
- 提交说明：Close UI renderer stage CD
