# STM32F103 ST7789 轻量 UI 工程

这是一个基于 STM32F103C8T6、ST7789 240x240 SPI 彩屏和 5D 摇杆的裸机轻量 UI 项目。

当前仓库的主线目标是：在不使用 LVGL、不使用动态内存、不使用全帧缓冲的前提下，实现可读、可操作、可继续扩展的嵌入式 UI。

## 当前状态

当前构建是 **UI-first 版本**：

- 已接入 5D 摇杆按键输入。
- 已完成 HOME / PLAYER / SETTINGS / INFO 页面框架。
- 已实现脏矩形局部刷新。
- 已实现 16x16 中文子集字库和放大 ASCII 字体。
- 已实现 HOME / SETTINGS 焦点条动画。
- 已修复焦点条抽动问题。
- GIF 播放源码和历史文档保留，但 GIF 数据和播放模块当前不参与 Keil 构建。

当前最新验证构建结果：

```text
0 Error(s), 0 Warning(s)
Code=12152
RO-data=1984
RW-data=16
ZI-data=2424
```

## 硬件

### ST7789

| 功能 | STM32F103C8T6 | 说明 |
| --- | --- | --- |
| SCK | PB13 | SPI2 SCK |
| MOSI | PB15 | SPI2 MOSI |
| RES | PB10 | LCD 复位 |
| DC | PB14 | 命令/数据选择 |
| BLK | PB12 | 背光控制 |
| CS | 无 | 当前屏幕模块无外部 CS |

注意：

- SCL/SDA 在该屏幕上是 SPI 信号，不是 I2C。
- 屏幕无 MISO，STM32 只向屏幕发送数据。
- 当前驱动配置为 240x240、SPI Mode 3、无 CS。

### 5D 摇杆

5D 摇杆按 **7 个共地数字按键** 使用，不需要 SPI、ADC 或屏幕接口。

| 摇杆功能 | STM32F103C8T6 | 配置 |
| --- | --- | --- |
| COM | GND | 公共地 |
| UP | PA0 | 输入上拉 |
| DOWN | PA1 | 输入上拉 |
| LEFT | PA2 | 输入上拉 |
| RIGHT | PA3 | 输入上拉 |
| MID | PA4 | 输入上拉 |
| SET | PA5 | 输入上拉 |
| RST | PA6 | 输入上拉 |

按键电平：

```text
未按下：高电平
按下：低电平
```

保留接口：

| 功能 | 引脚 |
| --- | --- |
| USART1 TX | PA9 |
| USART1 RX | PA10 |
| SWDIO | PA13 |
| SWCLK | PA14 |

不要占用 PA13/PA14，否则会影响 ST-Link 下载和调试。

## 软件架构

```text
key_driver
    10 ms 扫描、20 ms 消抖、长按、连发、RST 复位
        ↓
ui_event
    固定环形事件队列，物理按键转 UI 事件
        ↓
app_ui
    每轮处理有限数量事件，避免连发阻塞绘制
        ↓
ui_page
    页面路由、全局事件、页面 task、脏区绘制
        ↓
page_home / page_player / page_settings / page_info
        ↓
ui_draw / ui_font
    中文子集、大号 ASCII、菜单行、状态栏、页脚、控件绘制
        ↓
ui_dirty
    脏矩形队列、裁剪、合并、动画窄脏区
        ↓
bsp_st7789
    ST7789 SPI/DMA 底层驱动
```

## 当前页面

### HOME

中文大字号菜单：

- 动画播放器
- 设置
- 系统信息

UP/DOWN 移动焦点，MID/RIGHT 进入。

### PLAYER

当前是播放器占位页：

- 显示 GIF 禁用中。
- 保留播放/暂停/停止状态机。
- 后续恢复 GIF 时应只作为 PLAYER 页内部组件接入。

### SETTINGS

当前设置项：

- 动画：开/关
- 亮度：1..5
- 主题：CYAN / GOLD / GREEN

LEFT 返回，RIGHT/MID 修改当前项。

### INFO

显示 MCU、LCD、按键、GIF 状态和当前构建状态。

## 关键实现

### 不使用动态内存

项目不使用 `malloc()` / `free()`。事件队列、脏矩形队列、动画状态均为固定静态存储。

示例：

```c
static UI_Event items[16];
static UI_Rect rects[8];
static UI_FocusAnim g_home_focus_anim;
```

### 不使用全帧缓冲

240x240 RGB565 全屏 framebuffer 需要：

```text
240 x 240 x 2 = 115200 字节
```

STM32F103C8T6 RAM 只有约 20 KB，因此不能保存整屏图像。当前策略是：

```text
哪里变化，就刷新哪里
```

### 脏矩形刷新

普通 UI 变化使用 `UI_DirtyAdd()`，会裁剪并合并相近区域。动画焦点条使用 `UI_DirtyAddIsolated()`，避免窄条脏区被合并成整行刷新。

焦点条抽动修复后的核心原则：

```text
视觉上只有 5px 焦点条在动
实际刷新也必须只有 5px 焦点条区域
```

## 目录说明

```text
Project/
    Keil MDK 工程文件

User/
    应用层、UI、按键、ST7789 驱动

SYSTEM/
    delay、timing 等系统辅助模块

Libraries/
    STM32F10x 标准外设库

Tools/
    GIF 转换等辅助脚本

docs/
    学习文档、历史排障文档、变更日志
```

重点文档：

- [UI 开发学习文档](docs/ui-development-learning-guide.md)
- [ST7789 逻辑分析仪排障指南](docs/st7789-logic-analyzer-troubleshooting-guide.md)
- [ST7789 差分刷新优化学习笔记](docs/st7789-delta-optimization-learning.md)
- [历史 GIF 动画实现说明](docs/spi_oled_gif_animation.md)

## 编译

使用 Keil MDK 打开：

```text
Project/led.uvprojx
```

步骤：

1. 打开 Keil 工程。
2. 执行 `Rebuild`。
3. 确认 `0 Error(s), 0 Warning(s)`。
4. 使用 ST-Link 下载。
5. 复位开发板。

当前主循环入口：

```c
int main(void)
{
    ...
    Key_Init();
    App_UI_Init();

    while (1)
    {
        now = Timing_GetTick();
        Key_Task(now);
        App_UI_Task(now);
    }
}
```

## 当前按键语义

| 按键 | 功能 |
| --- | --- |
| UP | 上移焦点 |
| DOWN | 下移焦点 |
| LEFT | 返回 |
| RIGHT | 进入 / 修改 |
| MID | 确认 / 修改 |
| SET | 打开设置 |
| RST 短按 | 返回 |
| RST 长按约 600 ms | 回主页 |
| RST 持续约 2 秒 | 软件复位 |

## 后续计划

建议下一步优先继续打磨 UI，而不是立刻恢复 GIF：

1. SETTINGS 亮度进度条只刷新进度条区域。
2. HOME 进入页面增加短 pressed 反馈。
3. 页脚文案进一步压缩，避免中文拥挤。
4. PLAYER 页作为独立组件恢复 GIF 播放。
5. 评估 GIF 数据重新加入后的 Flash 余量。

恢复 GIF 时的原则：

```text
GIF 只属于 PLAYER 页面
离开 PLAYER 停止 GIF task
进入 PLAYER 重置 GIF 状态
HOME / SETTINGS / INFO 不依赖 GIF
```
