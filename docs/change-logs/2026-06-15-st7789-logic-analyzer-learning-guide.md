# ST7789 逻辑分析仪排障学习文档

日期：2026-06-15

## 修改目标

将本次 ST7789 点屏、逻辑分析仪抓包、SPI Mode 选择和 240x320
分辨率修正过程整理成可复用的中文学习文档。

## 修改前行为

工程中已有按时间记录的变更日志，但信息分散，缺少一份从基本波形检查到
ST7789 命令和 RGB565 数据判读的系统教程。

## 修改后行为

新增独立学习指南，记录最终实测配置、故障演进、PulseView 操作、
逻辑分析仪能力边界、SPI 模式原理、常见论坛问题和标准排障顺序。

## 逻辑变化范围

代码逻辑与原提交完全一致，仅新增 Markdown 学习文档和本次变更记录。

## 涉及文件

- `docs/st7789-logic-analyzer-troubleshooting-guide.md`
- `docs/change-logs/2026-06-15-st7789-logic-analyzer-learning-guide.md`

## 接口与兼容性

不修改固件接口、引脚、编译配置或生成文件。文档明确最终配置仅适用于本次
实测的 240x320、SPI Mode 3 模块。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 文档内容已与当前 `User/bsp_st7789.h` 和历史 ST7789 变更日志核对。
- 关键命令和颜色数据已与 `Output/logic` 中的实测抓包核对。
- 外部资料链接已于 2026-06-15 检查。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：this commit
- 提交说明：Add ST7789 logic analyzer troubleshooting guide
