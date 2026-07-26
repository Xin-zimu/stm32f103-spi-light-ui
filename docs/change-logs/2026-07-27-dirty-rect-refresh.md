# 脏矩形局部刷新

日期：2026-07-27

## 修改目标

实现阶段五“脏矩形局部刷新”：页面静态内容首次进入仍可整页绘制，但焦点移动、设置值变化、播放器状态变化只刷新变化区域，减少整屏清屏带来的割裂感。

## 修改前行为

UI 页面管理器只有一个整页重画标志。HOME、SETTINGS、PLAYER 的任何状态变化都会调用整页刷新，页面会先清全屏再重画所有控件。

## 修改后行为

新增固定容量脏矩形队列。页面切换和首次进入使用全屏脏区，页面内部的焦点和值变化只加入对应控件区域。绘制层增加当前裁剪矩形，所有矩形和文字绘制都会和脏区相交后再写 ST7789。

## 逻辑变化范围

- 新增 `ui_dirty` 模块，负责脏区裁剪、合并、溢出转全屏和逐个弹出。
- `UI_PageOps.draw` 改为接收 `const UI_Rect *clip`，页面任务每次处理一个脏区。
- `ui_draw` 增加绘制裁剪和局部清屏接口，矩形和文字按当前脏区裁剪。
- HOME/SETTINGS 焦点移动只标记旧行和新行，设置值变化只标记当前行。
- PLAYER 播放状态和进度变化只标记状态区域。
- UI 事件队列记录物理按键事件类型，队列满时优先用关键导航事件替换低优先级重复/方向事件。

## 涉及文件

- `User/ui_dirty.c`
- `User/ui_dirty.h`
- `User/ui_page.c`
- `User/ui_page.h`
- `User/ui_draw.c`
- `User/ui_draw.h`
- `User/page_home.c`
- `User/page_settings.c`
- `User/page_player.c`
- `User/page_info.c`
- `User/ui_event.c`
- `User/ui_event.h`
- `Project/led.uvprojx`

## 接口与兼容性

内部 UI 页面接口变化：页面绘制函数从无参数改为 `const UI_Rect *clip` 参数。外部应用入口、按键 GPIO、ST7789 引脚和 GIF 禁用状态不变。没有引入动态内存，也没有引入全帧缓冲。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，13 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- Keil isolated build：0 error(s), 0 warning(s)。
- 程序体积：Code=10352, RO-data=560, RW-data=16, ZI-data=2352。

## Git

- 分支：master
- 起始提交：472062942847128e1865e216044ec23d3ace8804
- Commit：this commit
- 提交说明：Add dirty rectangle UI refresh
