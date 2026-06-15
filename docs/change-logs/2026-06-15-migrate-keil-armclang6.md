# Keil工程迁移到Arm Compiler 6

日期：2026-06-15

## 修改目标

将Keil工程从仅以Lite状态运行的ARM Compiler 5.06迁移到已授权的
Arm Compiler 6.24，解除32 KB链接限制，并恢复小猫GIF全部28帧。

## 修改前行为

工程选择ARMCC 5.06。虽然Keil配置中存在标准版许可证信息，但ARMCC 5链接器
自报`MDK-ARM Lite 5.43`，镜像超过32 KB时报告L6047U错误。

为了适应该限制，动画只能保留9帧或10帧，无法按原GIF的40 ms节拍播放。

## 修改后行为

工程明确选择`V6.24::ARMCLANG`。旧CMSIS的4个naked栈指针函数改为AC6接受的
纯汇编函数体。生成器保留全部28帧，按40 ms间隔播放，完整循环仍使用帧差。

## 逻辑变化范围

- Keil目标编译器由ARMCC 5.06切换为ArmClang 6.24。
- C编译选项增加`-fgnu`，兼容旧标准外设库和CMSIS的GNU扩展。
- `core_cm3.c`中4个naked函数改为只含汇编指令，直接遵循r0调用约定。
- GIF生成帧数由9帧增加到全部28帧，间隔由125 ms恢复为40 ms。
- 重新生成首帧、28个循环帧差和偏移表。
- 更新README和GIF实现说明。

## 涉及文件

- `Project/led.uvprojx`
- `Libraries/CMSIS/core_cm3.c`
- `Tools/generate_st7789_gif.py`
- `User/anim_frames.c`
- `User/anim_frames.h`
- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-15-migrate-keil-armclang6.md`

## 接口与兼容性

接线、SPI Mode 3、18 MHz SPI、240x240尺寸和帧差协议不变。
工程现在要求本机安装Arm Compiler 6.24。最终加载镜像接近64 KB上限，
后续增加代码或图片前必须重新检查Flash余量。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- ARMCC 5链接器自报`MDK-ARM Lite 5.43`。
- ArmClang 6.24链接器自报`MDK Plus 5.43`。
- AC6首次构建发现旧CMSIS naked函数4个兼容错误，修复后通过。
- 完整28帧生成结果：首帧7200字节，循环帧差49109字节。
- 每个帧差通过生成脚本反向解码一致性检查。
- Keil AC6隔离构建通过：0个错误、0个警告。
- 程序大小：Code=4928，RO-data=56728，RW-data=4，ZI-data=1044字节。
- Flash加载量为61660字节，超过32 KB且低于64 KB上限，剩余约3876字节。
- Keil下载通过：`Erase Done`、`Programming Done`、`Verify OK`。
- 固件已启动，待用户确认实屏28帧播放效果。

## Git

- 分支：master
- 起始提交：c07eafc4e74e3c655ed26c32a23bfb2ef1ce2d50
- Commit：this commit
- 提交说明：Migrate project to Arm Compiler 6
