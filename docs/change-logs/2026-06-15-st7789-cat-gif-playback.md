# ST7789 小猫 GIF 播放

日期：2026-06-15

## 修改目标

将本地120x120小猫GIF转换为可装入当前Keil 32 KB镜像限制的动画数据，
在240x240 ST7789上循环播放，并恢复基于毫秒时基的非阻塞帧等待。

## 修改前行为

固件每秒阻塞切换两张人工生成的静态测试图；工程未编译TIM3时基和动画任务，
也没有读取真实GIF内容。

## 修改后行为

固件循环播放小猫GIF抽取的10帧16色动画，每帧保持120 ms。首帧完整写屏，
后续帧只更新相邻帧中发生变化的像素；等待下一帧期间主循环不阻塞。

## 逻辑变化范围

- 新增GIF合成、全局16色量化、帧差编码和反向校验生成脚本。
- 动画数据由两张整帧测试图改为一个完整首帧和9个帧差。
- ST7789驱动新增4位索引帧差应用接口。
- 启用TIM3 1 ms时基和非阻塞动画任务。
- 主循环改为持续调度动画任务，不再调用帧间阻塞延时。
- Keil加入TIM驱动、时基和动画任务。
- 更新README及动画格式学习文档。

## 涉及文件

- `Project/led.uvprojx`
- `SYSTEM/timing/timing.c`
- `Tools/generate_st7789_gif.py`（新增）
- `User/anim_frames.c`
- `User/anim_frames.h`
- `User/app_st7789_anim.c`
- `User/bsp_st7789.c`
- `User/bsp_st7789.h`
- `User/main.c`
- `README.md`
- `docs/spi_oled_gif_animation.md`
- `docs/change-logs/2026-06-15-st7789-cat-gif-playback.md`

## 接口与兼容性

接线、SPI Mode 3、240x240尺寸和偏移0不变。新增的帧差接口要求源图放大后
恰好覆盖屏幕，且输入流必须按每行120像素完整编码。TIM3现在用于动画时基。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- 原GIF检查：120x120、28帧、40 ms/帧、总时长1120 ms。
- 生成结果：10帧、120 ms/帧、首帧7200字节、帧差21099字节。
- 每个帧差均通过生成脚本反向解码一致性检查。
- 生成脚本重复执行结果一致。
- 工程XML解析和`git diff --check`通过。
- 涉及的C/H、Python和工程文件通过编码及无BOM检查。
- 新增及修改函数通过函数头注释检查。
- Keil隔离构建通过：0个错误、0个警告。
- 程序大小：Code=3106，RO-data=28670，RW-data=36，ZI-data=1028字节。
- 最终加载镜像约31812字节，低于Keil免费链接器32768字节上限。
- Keil下载通过：`Erase Done`、`Programming Done`、`Verify OK`。
- 固件已启动；下载后用户要求提交并推送，未报告实屏显示异常。

## Git

- 分支：master
- 起始提交：df1de4c5dc8d2efe00dc0e450c019a858cc1e1aa
- Commit：this commit
- 提交说明：Play compressed cat GIF on ST7789
