# UI 框架与页面模块

日期：2026-07-27

## 修改目标

按阶段 5 计划重做 UI 基础结构，把上一版集中在 `app_ui.c` 的页面、字体、绘制和事件分发拆成可扩展模块。当前继续保持 GIF 禁用，优先让 HOME、PLAYER、SETTINGS、INFO 四个页面具备清晰职责和统一绘制风格。

## 修改前行为

上一版已经临时移除 GIF 构建并显示 UI-first 页面，但大部分 UI 字体、绘制、页面状态、页面事件处理都堆在 `app_ui.c` 内。页面之间没有统一页面接口，绘制控件也没有复用边界，后续继续扩展会变成单文件堆逻辑。

## 修改后行为

新增 UI 框架模块：

- `ui_types.h`：集中定义 UI 页面 ID、矩形、屏幕布局和颜色常量。
- `ui_font.c/.h`：集中管理 ASCII 字模数据。
- `ui_draw.c/.h`：封装清屏、矩形、边框、文本、状态栏、页脚、菜单行、进度条和开关。
- `ui_page.c/.h`：维护当前页面、全局事件优先级、页面跳转和脏页重绘。
- `page_home.c/.h`：HOME 菜单页，支持 PLAYER / SETTINGS / INFO 三项选择。
- `page_player.c/.h`：播放器占位页，显示 GIF DISABLED、状态和进度条。
- `page_settings.c/.h`：设置页，支持 ANIMATION、BRIGHT、THEME 三项 RAM 设置。
- `page_info.c/.h`：信息页，显示 MCU、LCD、KEY、GIF、BUILD 状态。

`app_ui.c` 现在只负责从 UI 事件队列取事件并交给 `ui_page`，不再直接包含页面绘制和页面业务。

## 逻辑变化范围

- `app_ui.c` 从页面实现文件重构为 UI 应用入口。
- `ui_event.c/.h` 恢复 `source_key` 和 `timestamp` 字段，方便后续调试和统计。
- 新增统一绘制控件，页面不再直接大量调用 `ST7789_FillRect()`。
- 新增页面管理器，统一处理 `SYSTEM_RESET`、`HOME`、`SETTINGS`、`BACK` 等全局事件。
- HOME / PLAYER / SETTINGS / INFO 拆成独立页面模块。
- `Project/led.uvprojx` 加入新增 UI 模块文件。GIF 相关文件仍保持不参与当前构建。

## 涉及文件

- `User/app_ui.c`
- `User/ui_event.c`
- `User/ui_event.h`
- `User/ui_types.h`
- `User/ui_font.c`
- `User/ui_font.h`
- `User/ui_draw.c`
- `User/ui_draw.h`
- `User/ui_page.c`
- `User/ui_page.h`
- `User/page_home.c`
- `User/page_home.h`
- `User/page_player.c`
- `User/page_player.h`
- `User/page_settings.c`
- `User/page_settings.h`
- `User/page_info.c`
- `User/page_info.h`
- `Project/led.uvprojx`
- `docs/change-logs/2026-07-27-ui-framework-pages.md`

## 接口与兼容性

- 对主循环接口保持不变：`App_UI_Init()`、`App_UI_Task(uint32_t now)`。
- 对按键驱动接口保持不变：`Key_Init()`、`Key_Task(uint32_t now)`。
- 当前固件仍为 UI-first 构建，不编译 GIF 数据和 GIF 播放模块。
- ST7789 硬件引脚不变。
- 5D 摇杆 PA0..PA6 引脚和按键语义不变。
- 页面刷新仍为整屏重绘，脏矩形和动画留到后续阶段。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，19 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过；仅有 Git 换行符提示，无空白错误。
- Keil isolated build：通过。
  - 工程：`Project\led.uvprojx`
  - 输出：`Output\codex-verify-20260727-021355-108`
  - 结果：0 Error(s), 0 Warning(s)
  - 大小：`Code=10104 RO-data=560 RW-data=8 ZI-data=2288`

## Git

- 分支：master
- 起始提交：382026c478c313f47700d5c10875b27b4af4d515
- Commit：this commit
- 提交说明：Add modular UI page framework
