# 修复焦点条抽动

日期：2026-07-27

## 修改目标

修复阶段七后焦点条动画抽动问题。聚焦焦点条本身的刷新面积和按键连发时的动画起点，不改页面布局、中文字体或 GIF 状态。

## 修改前行为

焦点条每一帧返回的脏区宽度为 216 像素，并覆盖旧位置和新位置之间的大块菜单区域。页面绘制虽然有裁剪，但仍会在该大区域内重画行背景和部分文字。按键连发时，新动画从旧行位置重新开始，焦点条可能出现回跳。

## 修改后行为

焦点条动画脏区缩小为 5 像素宽，只覆盖旧条和新条的位置。动画脏区使用独立加入方式，避免被合并回整行刷新。按键连发时，新动画从当前焦点条位置继续滑向目标行。

## 逻辑变化范围

- `ui_anim` 将焦点条脏区从整行宽度改为焦点条窄条区域。
- `ui_anim` 在动画未结束时重新启动，会使用当前焦点条位置作为新起点。
- `ui_dirty` 增加 `UI_DirtyAddIsolated()`，用于动画条窄脏区，跳过普通合并逻辑。
- HOME 和 SETTINGS 的焦点条动画脏区改用独立脏区入口。
- 修正 `page_settings.c` 中两个函数注释的参数描述。

## 涉及文件

- `User/ui_anim.c`
- `User/ui_dirty.c`
- `User/ui_dirty.h`
- `User/page_home.c`
- `User/page_settings.c`

## 接口与兼容性

新增内部接口 `UI_DirtyAddIsolated()`。页面入口、按键映射、ST7789 引脚、GIF 禁用状态和已有中文字库不变。没有引入动态内存或全帧缓冲。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，5 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- Keil isolated build：0 error(s), 0 warning(s)。
- 程序体积：Code=12152, RO-data=1984, RW-data=16, ZI-data=2424。

## Git

- 分支：master
- 起始提交：2e913142541575b9a7a8868967be50a665eb46a0
- Commit：this commit
- 提交说明：Fix focus marker jitter
