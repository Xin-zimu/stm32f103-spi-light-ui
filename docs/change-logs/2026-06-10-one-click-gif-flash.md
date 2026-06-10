# 一键选择 GIF 编译烧录

日期：2026-06-10

## 修改目标

把原有需要输入命令和参数的 GIF 转换过程简化为双击文件、选择 GIF，然后自动转换、
编译和烧录的流程。

## 修改前行为

用户需要手工执行 Python 命令生成 `anim_frames.c/.h`，再打开 Keil 执行 Rebuild
和 Download。命令参数、输出路径和操作顺序容易出错。

## 修改后行为

双击根目录 `一键更换GIF并烧录.bat` 后弹出 GIF 文件选择框。工具固定使用最多
20 帧、阈值 128、100 ms/帧，自动覆盖动画数组，调用 Keil 重编译，并在 0 个
编译错误后使用工程配置的 ST-Link 下载到开发板。

## 逻辑变化范围

- 新增双击批处理入口。
- 新增中文弹窗工具，负责依赖检查、GIF 转换、Keil 重编译和下载。
- 缺少 Pillow 时可自动安装。
- 转换、编译或下载失败时停止后续步骤并显示日志位置。
- 不修改 STM32 驱动、动画调度和硬件引脚逻辑。

## 涉及文件

- `一键更换GIF并烧录.bat`
- `Tools/simple_gif_flash.py`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-10-one-click-gif-flash.md`

## 接口与兼容性

固件 API 和动画数组格式不变。工具面向 Windows、Python 3、Keil5，默认查找
`D:\keil5\core\UV4\UV4.exe`。烧录使用 Keil 工程中已经配置的下载器。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- Python AST 语法解析：通过。
- 实际 `Assets/oled_demo.gif` 转换：通过，识别 12 帧。
- Keil 命令行正常重编译：0 error(s)、0 warning(s)。
- 烧录失败分支模拟：正确停止并显示错误，不误报成功。
- 实际硬件烧录：当前未连接 ST-Link，未执行。

## Git

- 分支：master
- 起始提交：a2c3c755a7d746405e9057d6fbfe547ec5182381
- Commit：this commit
- 提交说明：`Add one-click GIF build and flash tool`
