# 更新 ST7789 当前阶段 README

日期：2026-06-14

## 修改目标

更新 README，使接线、SPI 配置、纯色测试和排查步骤与当前实际编译代码一致。

## 修改前行为

README 仍描述 PA5/PA7 软件 SPI、PA3/PA4 控制信号、背光直连 3.3V 和
四色循环，与当前 GPIOB 硬件 SPI2 驱动不一致。文档还包含当前代码不存在的
状态返回、超时恢复和写满 240x320 显存等说明。

## 修改后行为

README 明确 PB13/PB15/PB10/PB14/PB12 接线、无 CS 模块约束、Mode 0/3
切换方式、240x240 地址窗口和五色循环验收。文档同时给出 Keil 编译烧录步骤、
背光亮但无图像的排查顺序，以及纯色成功后的图片和动画计划。

## 逻辑变化范围

代码逻辑与修改前完全一致，仅更新项目说明文档。

## 涉及文件

- `README.md`
- `docs/change-logs/2026-06-14-st7789-readme-current-stage.md`

## 接口与兼容性

没有修改 C/H 接口、引脚配置或固件行为。README 现在记录当前接口：
PB13=SCL、PB15=SDA、PB10=RES、PB14=DC、PB12=BLK，无外部 CS。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- README 内容与 `User/bsp_st7789.h`、`User/bsp_st7789.c`、`User/main.c`
  和 `Project/led.uvprojx` 逐项核对。
- STM32 文本策略检查：2 个 Markdown 文件通过。
- `git diff --check`：通过。
- 旧 PA 引脚、软件 SPI、800 ms、240x320 和 `ST7789_Status` 描述检查：README
  中均不存在。
- 隔离 Keil 构建：0 Error，0 Warning。
- 程序大小：Code=1976，RO-data=308，RW-data=24，ZI-data=1024。
- 构建日志：`Output/codex-verify-20260614-190703-665/led.build_log.htm`。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：未创建；README 在本次会话开始前已有未提交修改，自动提交保护不允许混合提交。
- 提交说明：Update ST7789 current stage README
