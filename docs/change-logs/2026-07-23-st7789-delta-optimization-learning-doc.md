# ST7789 差分优化学习文档

日期：2026-07-23

## 修改目标

新增一份学习向 Markdown 文档，系统说明 ST7789 GIF 差分刷新优化方法：
差分流、绘制段统计、短空洞合并、`merge_gap` 参数取舍，以及这套方法对
不同 GIF 和不同屏幕方案的通用性。

## 修改前行为

项目已有实现说明和变更记录，但内容偏工程结果，没有集中解释算法思想、
指标含义、参数选择方法和换 GIF 时的适用边界。学习者需要在脚本、README
和代码之间来回查找。

## 修改后行为

新增 `docs/st7789-delta-optimization-learning.md`，从学习角度解释：

- 为什么 STM32 不直接解析 GIF。
- 当前首帧完整保存、后续帧差分保存的原因。
- 跳过段、绘制段和 ST7789 地址窗口之间的关系。
- 为什么绘制段数量会影响 DMA 刷新性能。
- 短空洞合并的原理和 `merge_gap` 的取舍。
- 如何解读 `delta_bytes`、`draw_runs`、`dma_bytes` 等统计字段。
- 换 GIF 时哪些素材更适合差分优化。
- 换尺寸、换屏幕时哪些思想通用，哪些格式要重做。
- 推荐的实验路线和当前不建议优先做的方向。

README 和 `docs/spi_oled_gif_animation.md` 增加到学习文档的链接。

## 逻辑变化范围

代码逻辑与上一提交完全一致。本次仅新增和链接 Markdown 学习文档。

## 涉及文件

- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/st7789-delta-optimization-learning.md`
- `docs/change-logs/2026-07-23-st7789-delta-optimization-learning-doc.md`

## 接口与兼容性

无固件接口、脚本接口和 Keil 工程配置变化。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，检查 4 个路径。
- `git diff --check`：通过；仅出现 Git 换行提示，无空白错误。
- Keil 隔离构建：通过，0 Error(s)、0 Warning(s)。
  程序大小：Code=8444，RO-data=56748，RW-data=4，ZI-data=2788。

## Git

- 分支：master
- 起始提交：84250ed56f0c0389186ee21c25ba64f9442e4a15
- Commit：this commit
- 提交说明：Add ST7789 delta optimization learning guide
