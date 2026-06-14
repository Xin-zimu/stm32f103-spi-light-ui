# ST7789 240x320 方向颜色测试图

日期：2026-06-15

> 后续纠正：此版本按 240x320 放置底部标记。实屏只显示顶部标记和中下方红线，
> 从而证明屏幕实际可见区域为 240x240。最终布局见后续纠正记录。

## 修改目标

在纯色循环验证通过后，执行计划第 1 项：显示一张不占用大图片数组的
240x320 方向和颜色测试图，用于确认旋转、镜像、颜色顺序和完整覆盖。

## 修改前行为

主程序每秒循环显示红、绿、蓝、白、黑五种纯色。该方式可以确认通信和
全屏填充，但无法直观区分上下左右、镜像和局部裁剪。

## 修改后行为

上电初始化后绘制一张固定诊断图并保持显示。画面包含白色边框、四角不同颜色的
L 标记、RGB/CMY 色块、中心十字和三级灰阶。

## 逻辑变化范围

- 新增带屏幕边界裁剪的 `ST7789_FillRect()`。
- 新增 `App_ST7789_ShowDiagnosticPattern()`，通过矩形组合测试图。
- 主程序由五色循环改为绘制一次固定测试图。
- Keil 工程加入 `app_st7789_test.c/.h`。
- 更新 README 和诊断说明中的预期画面及验收标准。

## 涉及文件

- `Project/led.uvprojx`
- `User/main.c`
- `User/bsp_st7789.c`
- `User/bsp_st7789.h`
- `User/app_st7789_test.c`
- `User/app_st7789_test.h`
- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-15-st7789-diagnostic-pattern.md`

## 接口与兼容性

新增公开矩形填充接口，不修改已有函数签名、SPI 配置或接线。测试图固定针对
当前 240x320 可见区域，仍使用 SPI Mode 3。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 9 个涉及文件通过 STM32 文本编码策略检查。
- 新增函数注释策略检查通过。
- Keil 工程 XML 解析通过。
- `git diff --check` 通过。
- Keil 隔离构建通过：0 Error(s)，0 Warning(s)。
- 程序大小：Code=2482，RO-data=298，RW-data=24，ZI-data=1024。
- 构建日志：`Output/codex-verify-20260615-012027-377/led.build_log.htm`。
- 已更新标准输出 `Output/led.axf` 和 `Output/led.hex`。
- Keil 下载输出确认：Erase Done、Programming Done、Verify OK、
  Application running。
- 人工实屏验收需对照 README 中的四角标记和色块布局确认。

## Git

- 分支：master
- 起始提交：34335ee0a78eef4f9b5037003f6820b10010e954
- Commit：this commit
- 提交说明：Add ST7789 diagnostic test pattern
