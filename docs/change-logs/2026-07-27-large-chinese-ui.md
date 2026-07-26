# 大字号中文界面

日期：2026-07-27

## 修改目标

执行阶段六：提升 UI 可读性并引入中文界面。保留阶段五脏矩形机制，继续关闭 GIF，把当前主要页面从英文调试文本改成更大的中文菜单和状态显示。

## 修改前行为

UI 使用 5x7 点阵绘制在 8x12 字符格内，字号偏小；页面标题、菜单项、底部提示主要为英文；菜单行高度只有 36 像素，整体更像调试界面。

## 修改后行为

UI 增加 16x16 GB2312 中文子集字库和 2 倍放大的 ASCII 绘制。HOME、SETTINGS、PLAYER、INFO 页面切换到大字号中文标题、菜单项和底部提示，菜单行高度提升到 48 像素，信息行和状态文字同步放大。

## 逻辑变化范围

- `ui_font` 增加中文子集字模表和 GB2312 查字接口。
- `ui_draw` 增加大号 ASCII、GB2312 混排文本、大号中文菜单行和中文信息行。
- `ui_types` 调整状态栏、底部提示栏和菜单行尺寸，并增加低强调分隔线颜色。
- HOME 页面改为中文大菜单：动画播放器、设置、系统信息。
- SETTINGS 页面改为中文设置项：动画、亮度、主题，以及中文开关和底部提示。
- PLAYER 页面改为中文状态显示，继续显示 GIF 禁用中。
- INFO 页面改为中文标题和部分中文状态，硬件缩写仍保留 ASCII，避免额外扩充字库。

## 涉及文件

- `User/ui_font.c`
- `User/ui_font.h`
- `User/ui_draw.c`
- `User/ui_draw.h`
- `User/ui_types.h`
- `User/page_home.c`
- `User/page_settings.c`
- `User/page_player.c`
- `User/page_info.c`

## 接口与兼容性

新增内部绘制接口 `UI_DrawTextLarge()`、`UI_DrawTextCN()`、`UI_DrawMenuRowCN()`、`UI_DrawInfoRowCN()`。页面路由、按键映射、ST7789 引脚、GIF 禁用状态和外部应用入口不变。中文只覆盖当前 UI 所需子集，不引入完整 GB2312 字库和动态内存。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 中文字库覆盖脚本：页面实际使用的 28 个中文字符均有字模，缺失 0 个。
- `test-stm32-text-policy.ps1`：通过，9 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- Keil isolated build：0 error(s), 0 warning(s)。
- 程序体积：Code=10852, RO-data=1968, RW-data=16, ZI-data=2352。

## Git

- 分支：master
- 起始提交：b5063944bd7519317cb77a66d0e7d17498df8a9c
- Commit：this commit
- 提交说明：Add large Chinese UI text
