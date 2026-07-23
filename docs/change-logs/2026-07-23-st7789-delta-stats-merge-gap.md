# ST7789 差分统计和短空洞合并

日期：2026-07-23

## 修改目标

为 ST7789 GIF 生成脚本增加差分段统计和可配置短空洞合并策略。
先在 PC 端观察不同 `merge_gap` 对 Flash 帧差大小、绘制段数量和 DMA
像素字节数的影响，再决定是否调整固件动画数据。

## 修改前行为

`Tools/generate_st7789_gif.py` 固定桥接 1 个源像素的未变化空洞，但没有参数
可调，也不会输出段数、短段数量或 DMA 字节估算。判断 SPI/DMA 后续优化方向
只能靠经验，不能量化地址窗口数量与像素传输量的取舍。

## 修改后行为

脚本新增：

- `--merge-gap`：控制两个变化段之间允许合并的未变化源像素数量，默认 1，
  保持原有生成结果。
- `--compare-gaps`：一次比较多个 `merge_gap` 参数的统计结果。
- `--stats-only`：只输出统计，不改写 `anim_frames.c/.h`。
- `--small-run-limit`：定义统计中的短绘制段阈值，默认 4 个源像素。

默认运行仍会生成 `User/anim_frames.c/.h`，并额外打印当前参数的统计摘要。
使用 `--stats-only --compare-gaps 0,1,2,4` 可以评估候选参数而不改动固件数组。

## 逻辑变化范围

- 将原先硬编码的 1 像素桥接逻辑抽成 `merge_short_gaps()`。
- `encode_delta()` 返回编码字节和 `DeltaStats` 统计。
- 新增动画构建、统计格式化和 CLI 参数解析。
- 更新 ST7789 GIF 文档中的脚本用法和统计字段说明。
- 默认 `--merge-gap 1` 生成结果与修改前一致，未改动 `anim_frames.c/.h`。

## 涉及文件

- `Tools/generate_st7789_gif.py`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-07-23-st7789-delta-stats-merge-gap.md`

## 接口与兼容性

无固件接口变化。脚本无参数运行仍读取项目根目录 `小猫图.gif` 并写入
`User/anim_frames.c/.h`。默认 `--merge-gap 1` 保持原有编码策略。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `py -B -m py_compile Tools/generate_st7789_gif.py`：通过。
- `py -B Tools/generate_st7789_gif.py --stats-only --compare-gaps 0,1,2,4`：通过。
- `py -B Tools/generate_st7789_gif.py`：通过，默认 `merge_gap=1` 生成
  49109 字节帧差。
- `git diff -- User/anim_frames.c User/anim_frames.h`：无实际差异，确认默认
  参数保持现有固件数组不变。
- `test-stm32-text-policy.ps1`：通过，检查 3 个路径。
- `git diff --check`：通过；仅出现 Git 换行提示，无空白错误。
- Keil 隔离构建：通过，0 Error(s)、0 Warning(s)。
  程序大小：Code=8444，RO-data=56748，RW-data=4，ZI-data=2788。

本次统计样例：

```text
merge_gap=0: delta_bytes=56291, draw_runs=14049, skip_runs=17409, dma_bytes=329056, draw_source_pixels=41132, min_run=1, max_run=30, small_runs=11495
merge_gap=1: delta_bytes=49109, draw_runs=10101, skip_runs=13461, dma_bytes=360640, draw_source_pixels=45080, min_run=1, max_run=50, small_runs=6169
merge_gap=2: delta_bytes=45922, draw_runs=7704, skip_runs=11064, dma_bytes=398992, draw_source_pixels=49874, min_run=1, max_run=53, small_runs=3279
merge_gap=4: delta_bytes=44392, draw_runs=5353, skip_runs=8713, dma_bytes=462360, draw_source_pixels=57795, min_run=1, max_run=71, small_runs=1345
```

## Git

- 分支：master
- 起始提交：4dc5e96e0d7a276c8990bea17f9b0902bcb44058
- Commit：this commit
- 提交说明：Add ST7789 delta statistics and merge-gap option
