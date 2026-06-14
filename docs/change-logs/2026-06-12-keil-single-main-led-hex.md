# Keil 单一 main.c 与 led.hex 工作流

日期：2026-06-12

## 修改目标

将工程收敛为标准 Keil 使用方式：只打开现有 `led.uvprojx`，只编辑和编译
`User/main.c`，只生成并下载 `Output/led.hex`。

## 修改前行为

工程构建文件已引用 `User/main.c`，但 Keil 用户配置仍残留
`User/main_st7789.c`。README 要求用户区分多个手工复制的 ST7789 HEX，
`Output` 中也存在多个相似名称的固件。

## 修改后行为

Keil 工程文件和用户配置都引用 `User/main.c`。Rebuild 生成标准
`Output/led.hex`，Download 直接下载当前工程产物，不再要求用户选择额外 HEX。

## 逻辑变化范围

- 修正 `led.uvoptx` 中残留的旧入口路径。
- 保持 `led.uvprojx` 的输出目录为 `Output`、输出名为 `led`、HEX 生成为开启。
- 删除额外命名的 ST7789 HEX，避免烧录错误文件。
- 固件运行逻辑、GPIOA 接线和 ST7789 驱动逻辑不变。

## 涉及文件

- `Project/led.uvoptx`
- `README.md`
- `docs/change-logs/2026-06-12-keil-single-main-led-hex.md`
- 删除 `Output/st7789_gpioa_soft_spi.hex`
- 删除 `Output/st7789_minimal.hex`
- 删除 `Output/st7789_soft_spi.hex`

## 接口与兼容性

公开 C API 和硬件接线不变。用户操作接口统一为 Keil 的 Rebuild 和 Download，
标准构建产物统一为 `Output/led.hex`。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 文本规范检查：通过。
- `git diff --check`：通过，无空白错误。
- Keil 隔离构建：0 Error(s)，0 Warning(s)。
- 原始 `Project/led.uvprojx` 标准 Rebuild：0 Error(s)，0 Warning(s)。
- 标准构建日志确认 `compiling main.c`。
- 链接映射确认入口来自 `..\User\main.c` 和 `main.o`。
- 程序大小：Code=1624，RO-data=312，RW-data=24，ZI-data=1024。
- `Output` 顶层只有 `led.hex`，大小 5570 字节。
- `Output/led.hex` SHA256：
  `B4474A0C53CB339CC078683682CC7197308C16B1D6F3344BFC628C8B6D01635F`。

## Git

- 分支：master
- 起始提交：5314fefbf7013d9306282084d9526d1ac0c6ef06
- Commit：this commit
- 提交说明：Use single Keil main and led hex workflow
