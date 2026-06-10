# SPI OLED GIF 动画变更记录

## 目标和原有行为

原工程仅有 PB6/PB7 软件 I2C SSD1306 驱动，主循环显示光敏值并控制交通灯，没有
SPI OLED 驱动、全屏图片接口、动画任务或 GIF 转换工具。

本次目标是在不覆盖旧 I2C OLED 模块的前提下，新增 PB13/PB15 硬件 SPI2 OLED
驱动、非阻塞分阶段动画、两帧测试数据、电脑端 GIF 转换脚本和完整接线排错文档。

## 修改后行为

`SPI_OLED_ANIM_DEMO` 默认为 1。上电后初始化 SSD1306 SPI OLED，依次显示文字、
单张测试图片和两帧循环动画。设为 0 后恢复原有光敏传感器、交通灯和 I2C OLED
界面。动画任务使用 `Timing_GetTick()`，不使用播放延时。

## 预期逻辑变化

- 新增 SPI2 单向发送和 PB10/PB12/PB14 控制引脚初始化。
- 新增 SSD1306 复位、初始化、清屏、定位、测试文字和 1024 字节全屏刷新。
- 新增文字、单帧、循环播放三个非阻塞状态。
- 新增两张保存在 Flash 中的页格式测试帧。
- 新增 GIF 缩放、二值化、反色和页格式输出工具。
- 使用编译模式开关隔离新 SPI 动画与旧 I2C UI。

## 修改文件

- `.gitignore`
- `User/main.c`
- `User/bsp_spi_oled.c`
- `User/bsp_spi_oled.h`
- `User/app_anim.c`
- `User/app_anim.h`
- `User/anim_frames.c`
- `User/anim_frames.h`
- `Tools/gif_to_oled_frames.py`
- `Project/led.uvprojx`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-10-spi-oled-gif-animation.md`

## 接口和协议影响

新增 `OLED_SPI_*` API 和 `App_Anim_*` API，不修改旧 `OLED_*` I2C API。SPI 使用
Mode 0、8 位、MSB first、软件 CS。动画帧接口固定为 128x64、每帧 1024 字节。

## 编码和注释合规

新增和修改的 C/H 文件转换为代码页 936、无 BOM；Markdown、Python 和 uvprojx
保持 UTF-8、无 BOM。新增或修改函数均提供用途、参数、返回值和副作用说明。

## 测试和结果

- Python 无缓存语法解析和 `--help`：通过。
- GIF 示例转换：成功输出 2 帧，检查到 2048 个字节初始化值，帧间隔宏为 120 ms。
- STM32 文本策略：11 个修改路径全部通过。
- 新函数注释：逐个检查新增和修改函数，均有前置详细块注释。
- Keil 隔离编译：0 error(s)、0 warning(s)。
- 程序大小：Code=2938、RO-data=2358、RW-data=40、ZI-data=1224。
- 自动 Git 提交：运行环境拒绝 Git 锁文件写入，无法生成提交哈希；源文件和验证结果已完成。

## 提交信息

Commit: this commit

计划提交信息：`Add SPI OLED GIF animation support`
