# 轻量 UI 与 5D 摇杆输入

日期：2026-07-26

## 修改目标

按需求文档的第一阶段先接入 5D 摇杆输入与轻量 UI 调度：

- PA0..PA6 配置为共地按键输入上拉。
- 实现 10 ms 扫描、20 ms 消抖、方向键连发、RST 长按和两秒复位事件。
- 建立固定 UI 事件队列和首页/GIF 播放页的最小应用状态机。
- 保留现有 ST7789 GIF delta 播放能力，并增加 UI 可调用的重播与暂停切换接口。
- 保持 STM32F103C8 64 KB Flash 目标可以编译通过。

## 修改前行为

上电后直接初始化 ST7789 并循环播放 GIF。主循环只调用 `App_ST7789_AnimTask()`，没有 PA0..PA6 摇杆扫描、UI 首页、按键事件队列或通过按键控制 GIF 的能力。

## 修改后行为

上电后显示一个由矩形组成的轻量首页。首页包含两个可选区域：第一项进入 GIF 播放，第二项作为设置占位。

- UP/DOWN：在首页两项之间切换焦点。
- MID/RIGHT：在首页进入当前项；GIF 页中 MID 暂停/继续，RIGHT 从首帧重播。
- SET：回到首页并选中设置占位项。
- LEFT/RST 短按：从 GIF 页返回首页；在首页回到第一项。
- RST 长按约 600 ms：返回首页。
- RST 持续按住约 2 秒：调用 `NVIC_SystemReset()`。

## 逻辑变化范围

- 新增 `key_driver.c/.h`：使用 GPIOA PA0..PA6 输入上拉，按下为低电平，采用定时扫描和软件消抖，不使用 EXTI。
- 新增 `ui_event.c/.h`：固定 16 项环形队列，把物理按键事件转换为 UI 事件。为压入 64 KB，队列满时直接丢弃新事件并计数。
- 新增 `app_ui.c/.h`：实现最小 UI 状态机、首页矩形绘制、GIF 页输入控制和全局 RST/SET 处理。
- 修改 `app_st7789_anim.c/.h`：增加 GIF 重播和暂停切换接口；默认暂停，进入 GIF 页时重画首帧并恢复播放。
- 修改 `main.c`：主循环改为 `Key_Task(now)` + `App_UI_Task(now)`，不再开机自动播放 GIF。
- 修改 `Project/led.uvprojx`：加入新增 UI/按键文件。为保证 64 KB Flash 目标构建通过，本次从 Keil 编译列表移除未使用的 USART 源文件和 `fault_handlers.c`；这些源码文件仍保留在工程目录。

## 涉及文件

- `User/key_driver.c`
- `User/key_driver.h`
- `User/ui_event.c`
- `User/ui_event.h`
- `User/app_ui.c`
- `User/app_ui.h`
- `User/app_st7789_anim.c`
- `User/app_st7789_anim.h`
- `User/main.c`
- `Project/led.uvprojx`
- `docs/change-logs/2026-07-26-light-ui-joystick.md`

## 接口与兼容性

- 新增应用接口：`App_UI_Init()`、`App_UI_Task(uint32_t now)`。
- 新增按键接口：`Key_Init()`、`Key_Task(uint32_t now)`。
- 新增 GIF 控制接口：`App_ST7789_AnimRestart()`、`App_ST7789_AnimTogglePaused()`。
- ST7789 引脚保持不变：PB10 RES、PB12 BLK、PB13 SPI2 SCK、PB14 DC、PB15 SPI2 MOSI。
- 新增摇杆引脚：PA0 UP、PA1 DOWN、PA2 LEFT、PA3 RIGHT、PA4 MID、PA5 SET、PA6 RST，COM 接 GND。
- PA9/PA10 未被本次固件占用；USART 源码保留但不参与当前 64 KB 目标构建。
- `fault_handlers.c` 源码保留但不参与当前构建，异常处理回到启动文件默认处理方式。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，10 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过；仅有 Git 换行符提示，无空白错误。
- Keil isolated build：通过。
  - 工程：`Project\led.uvprojx`
  - 输出：`Output\codex-verify-20260726-015935-652`
  - 结果：0 Error(s), 0 Warning(s)
  - 大小：`Code=8800 RO-data=56728 RW-data=4 ZI-data=2236`

## Git

- 分支：master
- 起始提交：1e61f4972f7505d3ac7d475bd44d521b738f1cde
- Commit：this commit
- 提交说明：Add lightweight UI joystick input
