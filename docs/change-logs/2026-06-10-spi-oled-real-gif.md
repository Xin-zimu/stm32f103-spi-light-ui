# SPI OLED 真实 GIF 动画集成

日期：2026-06-10

## 修改目标

完成从实际 GIF 素材到 STM32 固件数组的最终集成，使工程烧录后不再只播放两张
手工测试图，而是循环播放 12 帧完整动画。

## 修改前行为

工程已有 SPI2 SSD1306 驱动、非阻塞动画任务和 GIF 转换脚本，但
`User/anim_frames.c/.h` 仅包含两张手工测试帧，工程目录中没有可重复转换的
实际 GIF 源文件。

## 修改后行为

工程内置 `Assets/oled_demo.gif`，分辨率为 128x64，共 12 帧，每帧 100 ms。
转换脚本已将该 GIF 生成 12288 字节页格式数组。烧录后先显示启动文字和首帧，
随后通过 `Timing_GetTick()` 循环播放移动方块、轨迹和进度条动画。

## 逻辑变化范围

- 将 `ANIM_FRAME_COUNT` 从 2 改为 12。
- 用实际 GIF 转换结果替换两帧测试数组。
- 转换脚本生成的 C/H 文件改为代码页 936、无 BOM，并添加中文帧和宏说明。
- 不修改 SPI 驱动、动画调度逻辑、硬件引脚或旧 I2C OLED 功能。

## 涉及文件

- `Assets/oled_demo.gif`
- `User/anim_frames.c`
- `User/anim_frames.h`
- `Tools/gif_to_oled_frames.py`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-10-spi-oled-real-gif.md`

## 接口与兼容性

`anim_frames` 数组接口和 128x64 页格式保持不变，仅帧数增加到 12。
动画数据约占 12 KB Flash，不占用 12 KB RAM。脚本仍兼容原有命令行参数。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- GIF 属性检查：128x64、12 帧、100 ms/帧。
- 数组长度检查：12288 个字节初始化值，等于 12 × 1024。
- Python 转换测试：语法解析、命令帮助、实际 GIF 转换全部通过。
- 文本编码检查：5 个本次修改的文本文件全部通过。
- 新函数注释检查：通过；本次生成的动画数据文件没有新增函数。
- Keil 隔离编译：0 error(s)、0 warning(s)。
- 程序大小：Code=2938、RO-data=12598、RW-data=40、ZI-data=1224。

## Git

- 分支：master
- 起始提交：b76237f4a3bb03394a9f13287a8d72af6a883b6c
- Commit：this commit
- 提交说明：`Integrate final SPI OLED GIF animation`
