# PLAYER 程序动画页

日期：2026-07-27

## 修改目标

把 PLAYER 页从静态“NO GIF / PROC ANIM”占位页升级为真正的程序生成动画验证页，不引入图片帧、大数组或真实 GIF 数据，用持续局部刷新验证 `ui_renderer` 条带双缓冲。

## 修改前行为

PLAYER 页只显示静态占位内容。按 OK/RIGHT 只改变状态文字和进度条，没有周期性页面任务，也不会持续产生动画 dirty，因此无法验证 renderer 在连续局部刷新下的行为。

## 修改后行为

PLAYER 页新增固定动画画布。PLAYING 状态下每 33 ms 推进一帧，并只将动画画布加入 dirty 队列。动画由扫描条、移动块和波形条通过矩形绘制生成，不存储任何帧图像。PAUSED 保持静态暂停画面，STOPPED 显示 `NO GIF` / `PROC ANIM`。RIGHT 从头播放，OK/MID 播放/暂停切换，LEFT 返回。

README 更新为当前状态，记录 PLAYER 已实现程序生成动画，并把下一阶段计划调整为 SETTINGS 局部刷新、pressed 反馈和更多程序图形。

## 逻辑变化范围

- `page_player.c` 增加动画画布常量、帧计数、上次步进时间。
- 新增 `Page_Player_GetAnimRect()`、`Page_Player_InvalidateAnim()`、`Page_Player_Task()`、`Page_Player_DrawAnim()`。
- `PAGE_PLAYER_OPS` 挂接 page task，让 PLAYING 状态可周期性推进动画。
- 状态变化时刷新状态区和动画画布；播放中每帧只刷新动画画布。
- 未新增动态内存、帧数组、GIF 数据或新的模块文件。

## 涉及文件

- `User/page_player.c`
- `README.md`

## 接口与兼容性

无公开接口变化。PLAYER 页面仍通过现有 `UI_PageOps` 接入页面管理，按键语义保持 LEFT 返回、RIGHT 从头播放、OK/MID 播放/暂停。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1 -Paths User\page_player.c, README.md`：通过。
- `test-new-function-comments.ps1 -Paths User\page_player.c`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=11740, RO-data=1988, RW-data=40, ZI-data=6384。
- 构建日志：`Output\codex-verify-20260727-184555-199\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：c94df3858ae23784d60db53208ac8949e2072e4f
- Commit：this commit
- 提交说明：Add PLAYER program animation
