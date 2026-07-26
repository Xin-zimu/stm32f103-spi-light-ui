# 临时移除 GIF 专注 UI

日期：2026-07-27

## 修改目标

根据实机反馈切换到 UI 优先验证路线：当前固件临时移除 GIF 数据和 GIF 播放模块，不再为了 64 KB Flash 压缩 UI 表现，集中实现可读、可操作的轻量 UI 页面。

## 修改前行为

上一阶段为了保留 GIF 数据，固件 Flash 几乎满载，只能显示很少的矩形 UI。首页没有可读文字，设置和信息页无法充分展开，实际观感接近占位验证。

## 修改后行为

当前构建不再编译 `anim_frames.c` 和 `app_st7789_anim.c`，释放原 GIF 数据占用的 Flash 空间。

新增 UI-first 页面：

- HOME：显示 `UI HOME`、`PLAYER`、`SETTINGS`、`INFO` 三个菜单项。
- PLAYER：显示播放器占位页、运行/暂停状态和按键提示。
- SETTINGS：显示 `ANIM` 开关和 `LIGHT` 等级条。
- INFO：显示 MCU、LCD、KEY、GIF OFF 等状态信息。

按键行为：

- UP/DOWN：菜单或设置项切换。
- MID/RIGHT：进入页面或改变当前设置。
- SET：从任意页面进入 SETTINGS。
- LEFT/RST 短按：返回 HOME。
- RST 长按约 600 ms：返回 HOME。
- RST 持续约 2 秒：软件复位。

## 逻辑变化范围

- `app_ui.c` 去掉 GIF 播放依赖，改为 UI-only 页面系统。
- 新增 5x7 ASCII 小字体绘制函数，使用 `ST7789_FillRect()` 画字，不引入大字库。
- `main.c` 去掉 `app_st7789_anim.h` 引用，主循环只负责按键扫描和 UI 调度。
- `Project/led.uvprojx` 从当前 Keil 构建列表移除 `anim_frames.c/.h` 与 `app_st7789_anim.c/.h`，源码文件仍保留，后续可重新加入。

## 涉及文件

- `User/app_ui.c`
- `User/main.c`
- `Project/led.uvprojx`
- `docs/change-logs/2026-07-27-ui-first-no-gif.md`

## 接口与兼容性

- ST7789 引脚保持不变：PB10 RES、PB12 BLK、PB13 SPI2 SCK、PB14 DC、PB15 MOSI。
- 5D 摇杆引脚保持不变：PA0..PA6 输入上拉，COM 接 GND。
- GIF 源码和数据文件未删除，只是从当前 Keil 构建列表临时移除。
- 当前固件是 UI 验证版，不播放 GIF。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过；仅有 Git 换行符提示，无空白错误。
- Keil isolated build：通过。
  - 工程：`Project\led.uvprojx`
  - 输出：`Output\codex-verify-20260727-014535-019`
  - 结果：0 Error(s), 0 Warning(s)
  - 大小：`Code=9048 RO-data=472 RW-data=8 ZI-data=2168`

## Git

- 分支：master
- 起始提交：e1ce403cfcce184064e4a0ed215949aaeddbb4e1
- Commit：this commit
- 提交说明：Switch to UI-first build without GIF
