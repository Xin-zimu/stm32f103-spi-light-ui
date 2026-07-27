# UI 组件完善

日期：2026-07-27

## 修改目标

一次性完成当前轻量 UI 组件完善计划：SETTINGS 亮度条局部刷新、HOME/PLAYER/SETTINGS pressed 反馈统一、PLAYER 动画帧计数增强，以及 INFO 页 renderer/dirty 调试统计显示。

## 修改前行为

SETTINGS 修改亮度会按整行刷新，无法验证更细粒度 dirty。HOME 按 OK/RIGHT 立即切页，当前行 pressed 反馈会被目标页全屏重绘吞掉。PLAYER 播放/暂停没有统一 pressed 反馈，也没有帧计数显示。INFO 页只显示静态系统信息，不能直接观察 renderer/dirty 统计。

## 修改后行为

SETTINGS 亮度变更只刷新进度条区域，并在短时间内显示高亮边框。动画开关和主题仍按整行刷新。

HOME 按 OK/RIGHT 时先显示当前行 pressed 反馈，约 60 ms 后再进入目标页；UP/DOWN/LEFT 会取消 pending enter。PLAYER OK/RIGHT 在状态区显示 pressed 反馈。

PLAYER 动画画布显示帧计数，播放时仍只刷新动画画布。INFO 页显示条带提交数、renderer 忙返回数、dirty 队列 max/current。

## 逻辑变化范围

- `page_settings.c` 新增亮度条矩形，亮度项变更改为局部 dirty 和进度条反馈边框。
- `page_home.c` 新增 pending enter 状态，使用非阻塞 task 延迟完成页面跳转以保留 pressed 反馈。
- `page_player.c` 新增状态区矩形、pressed 反馈、帧计数格式化和绘制。
- `page_info.c` 读取 `UI_RendererStats` 和 `UI_DirtyStats`，用轻量数字格式化显示调试数据。
- `README.md` 更新当前状态、页面说明、后续计划和构建尺寸。

## 涉及文件

- `User/page_settings.c`
- `User/page_home.c`
- `User/page_player.c`
- `User/page_info.c`
- `README.md`

## 接口与兼容性

无公开接口变化。继续使用已有 `UI_PageOps`、`UI_FeedbackPress()`、`UI_RendererGetStats()` 和 `UI_DirtyGetStats()`。HOME 进入页面新增约 60 ms 非阻塞反馈窗口，按键扫描和 renderer 不被阻塞。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1 -Paths User\page_settings.c, User\page_home.c, User\page_player.c, User\page_info.c, README.md`：通过。
- `test-new-function-comments.ps1 -Paths User\page_settings.c, User\page_home.c, User\page_player.c, User\page_info.c`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=12804, RO-data=1988, RW-data=40, ZI-data=6400。
- 构建日志：`Output\codex-verify-20260727-185338-818\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：b1636e67bb966ff48e25a2d1c8b15e6c238883ff
- Commit：this commit
- 提交说明：Polish UI components
