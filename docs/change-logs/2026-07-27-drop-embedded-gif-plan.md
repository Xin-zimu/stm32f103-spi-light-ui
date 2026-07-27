# 取消内置 GIF 计划

日期：2026-07-27

## 修改目标

取消当前主线的内置真实 GIF 计划，把项目方向明确为 STM32F103C8 上的轻量 UI 和程序生成动画。同步更新屏幕页面文案、README 和相关注释，避免后续继续围绕大 GIF 帧数据推进。

## 修改前行为

README 将下一阶段描述为 GIF 与 UI 共用刷新调度，并写有恢复 PLAYER 页 GIF 组件的计划。PLAYER 页显示“GIF 禁用中”，INFO 页显示 GIF 状态。`app_config.h` 还保留当前主线不再使用的 GIF catch-up 配置项。renderer/dirty 统计注释仍提到 GIF/UI 调度。

## 修改后行为

README 明确 STM32F103C8 Flash/RAM 不适合内置真实 GIF，历史 GIF 代码和文档只作为资料保留，不再作为当前主线计划。后续计划改为轻量 UI 组件和程序生成动画。

PLAYER 页改为轻量程序动画占位页，屏幕显示 `NO GIF` 和 `PROC ANIM`。INFO 页显示媒体策略为 `NO GIF`。主线配置删除未使用的 `APP_GIF_MAX_CATCHUP_FRAMES`。

## 逻辑变化范围

- 修改 PLAYER 页静态文案和注释，不改变页面状态机和按键行为。
- 修改 INFO 页媒体状态显示。
- 删除当前主线未使用的 GIF catch-up 宏。
- 更新 renderer/dirty 统计注释，将后续用途从 GIF/UI 调度改为 UI 动画调度。
- 更新 README 的当前状态、媒体边界和后续计划。

## 涉及文件

- `README.md`
- `User/page_player.c`
- `User/page_info.c`
- `User/app_config.h`
- `User/ui_dirty.c`
- `User/ui_renderer.c`

## 接口与兼容性

无公开函数接口变化。页面路由、按键语义、renderer/dirty 统计接口保持不变。历史 GIF 源文件仍保留在仓库中，但当前 Keil 主线不依赖它们。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1 -Paths README.md, User\page_player.c, User\page_info.c, User\app_config.h, User\ui_dirty.c, User\ui_renderer.c`：通过。
- `test-new-function-comments.ps1 -Paths User\page_player.c, User\page_info.c, User\app_config.h, User\ui_dirty.c, User\ui_renderer.c`：通过。
- README / 当前主线文件关键字检查：无“恢复 GIF”、“GIF 调度”、“GIF 与 UI”、“APP_GIF”旧计划残留。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=11308, RO-data=1984, RW-data=40, ZI-data=6376。
- 构建日志：`Output\codex-verify-20260727-183834-715\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：03ebff4e98f95ea99915c2dee1e160cb27d8beaf
- Commit：this commit
- 提交说明：Drop embedded GIF plan
