# STM32F103 ST7789 2048 新项目交接规划

日期：2026-07-30

## 1. 文档目的

这份文档用于下一个新项目启动时快速接续当前经验。

当前项目已经完成一个可运行的 STM32F103 + ST7789 轻量 UI 原型，但最终应用层主要还是动画演示和刷新链路验证。下一个项目建议不要继续在这个仓库里追加玩法，而是新开一个更聚焦的工程：基于同一硬件平台，做一个真正可玩的 2048 掌机小游戏。

本文档说明：

- 当前项目已经完成了什么。
- 当前项目的关键技术细节和限制。
- 哪些内容值得迁移到新项目。
- 哪些历史路线不要继续投入。
- 新项目的目标、架构、页面、玩法和阶段计划。
- 验收标准和风险点。

## 2. 当前项目结论

当前项目路径：

```text
D:\stm32f103实例\spi_oled DMAui
```

当前项目定位：

```text
STM32F103C8T6 + ST7789 240x240 SPI 彩屏 + 5D 摇杆的裸机轻量 UI 验证工程
```

当前项目最新主线状态：

```text
UI-first 版本
无 LVGL
无动态内存
无 240x240 全帧缓冲
条带式 RGB565 局部刷新
SPI2 TX DMA 送屏
```

当前项目已经证明：

- ST7789 240x240 屏幕可以稳定初始化和显示。
- SPI2 + DMA 可以承担局部刷新。
- 240x240 RGB565 全帧缓冲不适合 STM32F103C8T6。
- 用 240x4 条带 buffer 可以在 20 KB RAM 内实现页面绘制。
- 脏矩形 + 窄条带 DMA 可以支撑基础 UI。
- 5D 摇杆数字按键输入可用于菜单和交互。
- INFO 页统计能观察刷新压力。

当前项目的不足：

- 最终应用层主要是程序动画演示，不是真正有玩法的产品。
- UI 控件数量有限，更多是证明 renderer 可用。
- 没有高层游戏状态机、计分、胜负、重开等完整体验。
- 真实 GIF 路线已经确认不适合当前 MCU，不应作为新项目主线。

## 3. 当前硬件平台

### 3.1 MCU

```text
STM32F103C8T6
Flash：通常 64 KB
RAM：约 20 KB
```

设计约束：

- 不使用全屏 framebuffer。
- 不使用 `malloc()` / `free()`。
- 避免大图片帧、大字库和大量静态素材。
- 所有队列、游戏状态、绘制 buffer 使用固定静态内存。

### 3.2 LCD

屏幕：

```text
ST7789
240x240
RGB565
SPI 接口
```

当前接线：

| 功能 | STM32F103C8T6 | 说明 |
| --- | --- | --- |
| SCK | PB13 | SPI2 SCK |
| MOSI | PB15 | SPI2 MOSI |
| RES | PB10 | LCD 复位 |
| DC | PB14 | 命令/数据选择 |
| BLK | PB12 | 背光控制 |
| CS | 无 | 当前屏幕模块无外部 CS |

注意：

- 屏幕无 MISO，不能读屏幕显存。
- SCL/SDA 是 SPI 信号，不是 I2C。
- 当前驱动使用 SPI Mode 3。
- 240x240 面板存在偏移配置，沿用当前驱动即可。

### 3.3 5D 摇杆

当前 5D 摇杆按 7 个共地数字按键使用：

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

不要占用 PA13/PA14，否则影响 ST-Link 下载和调试。

## 4. 当前项目软件架构

当前项目的主链路：

```text
key_driver
    10 ms 扫描、20 ms 消抖、长按、连发
        ↓
ui_event
    固定环形事件队列，物理按键转 UI 事件
        ↓
app_ui
    每轮处理有限数量事件，避免输入阻塞绘制
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
    脏矩形队列、裁剪、合并、FIFO 消费、统计
        ↓
ui_renderer
    窄条带双缓冲、页面 buffer 绘制、LCD DMA 提交、统计
        ↓
bsp_st7789 / lcd_dma
    ST7789 + SPI2 TX DMA 底层驱动
```

这个架构对下一个项目仍然有价值，但建议新项目只迁移必要模块，不要把历史 GIF 代码一起带过去。

## 5. 当前项目值得迁移的模块

建议迁移：

- `User/app_config.h`
- `User/key_driver.c`
- `User/key_driver.h`
- `User/ui_event.c`
- `User/ui_event.h`
- `User/ui_types.h`
- `User/ui_dirty.c`
- `User/ui_dirty.h`
- `User/ui_renderer.c`
- `User/ui_renderer.h`
- `User/ui_draw.c`
- `User/ui_draw.h`
- `User/ui_font.c`
- `User/ui_font.h`
- `User/ui_feedback.c`
- `User/ui_feedback.h`
- `User/ui_page.c`
- `User/ui_page.h`
- `User/bsp_st7789.c`
- `User/bsp_st7789.h`
- `User/lcd_dma.c`
- `User/lcd_dma.h`
- `SYSTEM/delay`
- `SYSTEM/timing`
- `Libraries/STM32F10x_StdPeriph_Driver`

可参考但不必原样迁移：

- `page_home.c`
- `page_settings.c`
- `page_info.c`
- `page_player.c`

不建议迁移为新项目主线：

- `anim_frames.c`
- 历史 GIF 帧数据。
- 真实 GIF 播放相关工具和旧方案。

## 6. 当前 renderer 的关键经验

### 6.1 为什么不能全帧缓冲

240x240 RGB565 framebuffer 需要：

```text
240 x 240 x 2 = 115200 字节
```

STM32F103C8T6 RAM 约 20 KB，不可能长期保存整屏 framebuffer。

### 6.2 当前刷新策略

当前 renderer 使用：

```text
dirty rect
    ↓
按 4 像素高拆成 strip
    ↓
保留 dirty 的 x/w 窄窗口
    ↓
页面 draw 写入 strip buffer
    ↓
ST7789 设置 address window
    ↓
SPI2 TX DMA 发送 RGB565 数据
```

当前 buffer 数量：

```text
2 个 strip buffer
每个最大 240 x 4 x 2 = 1920 字节
合计 3840 字节
```

### 6.3 窄条带注意事项

窄条带不是只改 ST7789 窗口就够了。

必须同时保证：

- `strip.x` 和 `strip.w` 来自 dirty。
- `valid_length = strip.w * strip.h * 2`。
- `ui_draw` 写 buffer 时使用相对坐标：

```text
buffer_x = rect.x - strip.x
buffer_y = rect.y - strip.y
stride = strip.w
```

如果继续用屏幕绝对 x 坐标写入 buffer，会导致发送数据从 buffer 前端取错，屏幕显示错位。

### 6.4 INFO 页统计经验

INFO 页不能在每个 `draw()` 中读取实时统计。

原因：

- renderer 会按 strip 多次调用页面 draw。
- 同一个数字的不同横向切片可能来自不同计数值。
- 屏幕上会显示出拼接错误的数字。

正确做法：

- 进入 INFO 时抓取一次统计快照。
- 整页所有 strip 使用同一份缓存字符串。
- RIGHT 可刷新快照。
- OK/MID 可清零统计后显示清零快照。

## 7. 当前项目按键语义

当前主界面：

| 按键 | 功能 |
| --- | --- |
| UP | 上移 |
| DOWN | 下移 |
| LEFT | 返回 |
| RIGHT | 进入 / 修改 / 刷新 |
| MID | 确认 / 修改 / 清零 |
| SET | 进入设置 |
| RST 短按 | 返回 |
| RST 长按约 600 ms | 回主页 |
| RST 持续约 2 秒 | 软件复位 |

新项目建议继续使用这个基础语义，但游戏中要重新映射：

| 按键 | 2048 游戏建议 |
| --- | --- |
| UP | 上滑 |
| DOWN | 下滑 |
| LEFT | 左滑 |
| RIGHT | 右滑 |
| MID | 重新开始 / 确认 |
| SET | 暂停 / 菜单 |
| RST 短按 | 返回主菜单 |

## 8. 下一个项目目标

新项目建议名称：

```text
stm32f103_st7789_2048
```

新项目定位：

```text
基于 STM32F103C8T6、ST7789 240x240 SPI 彩屏和 5D 摇杆的裸机 2048 小游戏
```

核心目标：

- 做一个真正可玩的 2048。
- 保留当前项目的轻量 UI 和 DMA 局部刷新优势。
- 不使用 LVGL。
- 不使用动态内存。
- 不使用全屏 framebuffer。
- 让局部刷新服务于游戏体验，而不是只做演示动画。

最终体验：

- 开机进入主菜单。
- 选择开始游戏。
- 4x4 棋盘显示在 240x240 屏幕中。
- 使用摇杆四方向移动方块。
- 方块合并、加分、生成新块。
- 显示当前分数和最好分数。
- 检测胜利和失败。
- 支持重新开始。
- 支持设置页和诊断页。

## 9. 新项目页面规划

建议页面：

```text
HOME
GAME
PAUSE
SETTINGS
INFO
ABOUT
```

### 9.1 HOME

菜单项：

- 开始游戏
- 设置
- 系统信息
- 关于

### 9.2 GAME

显示：

- 4x4 棋盘。
- 当前分数。
- 最高分。
- 游戏状态：PLAY / WIN / GAME OVER。

按键：

- UP/DOWN/LEFT/RIGHT：移动棋盘。
- MID：游戏结束时重开；游戏中可打开确认菜单。
- SET：暂停菜单。
- RST 短按：返回 HOME。

### 9.3 PAUSE

菜单项：

- 继续
- 重新开始
- 返回主页

### 9.4 SETTINGS

建议设置项：

- 动画：开 / 关。
- 主题：CYAN / GOLD / GREEN。
- 新块动画：开 / 关。
- 合并动画：开 / 关。

不建议一开始做：

- 复杂音效。
- 存档。
- 多语言。
- 大量皮肤。

### 9.5 INFO

继续保留性能诊断：

- STRIP
- KB
- BUSY
- DMA
- DIRTY

按键：

- OK/MID：清零统计。
- RIGHT：刷新快照。
- LEFT：返回。

### 9.6 ABOUT

显示：

- 项目名。
- MCU。
- LCD。
- 输入方式。
- 简短说明。

## 10. 2048 游戏设计

### 10.1 数据结构

棋盘只需要 4x4：

```c
static uint16_t g_game_board[4][4];
```

建议存储实际数值：

```text
0, 2, 4, 8, 16, ... 2048
```

也可以存指数：

```text
0 表示空
1 表示 2
2 表示 4
3 表示 8
...
```

推荐存指数，原因：

- 数值范围小。
- 颜色表索引方便。
- 判断和绘制更轻量。

建议：

```c
static uint8_t g_game_board[4][4];
```

### 10.2 游戏状态

建议状态：

```text
GAME_STATE_READY
GAME_STATE_PLAYING
GAME_STATE_WIN
GAME_STATE_OVER
GAME_STATE_PAUSED
```

### 10.3 随机数

不要引入复杂随机库。

可使用一个轻量 LCG：

```text
seed = seed * 1103515245 + 12345
```

种子来源：

- 启动 tick。
- 首次按键时间。
- 按键事件 timestamp。

新块生成：

- 90% 生成 2。
- 10% 生成 4。

### 10.4 移动算法

每次移动：

1. 根据方向取出 4 条线。
2. 去掉 0。
3. 相邻相等则合并。
4. 合并后补 0。
5. 写回棋盘。
6. 如果棋盘有变化，生成一个新块。
7. 更新分数。
8. 标记变动格子的 dirty。
9. 检测胜利或失败。

不要一开始做复杂动画。先保证规则正确。

### 10.5 胜负判定

胜利：

```text
任意格子达到 2048
```

失败：

```text
没有空格
并且上下左右没有任何相邻相同格子
```

## 11. 2048 屏幕布局

240x240 建议布局：

```text
0..31      顶部状态栏：分数 / BEST
32..219    4x4 棋盘
220..239   底部提示栏
```

棋盘建议：

```text
棋盘 x = 16
棋盘 y = 42
棋盘 w = 208
棋盘 h = 168 或 176
```

如果使用正方形格子：

```text
cell = 48
gap = 4
board = 4 * 48 + 5 * 4 = 212
```

更紧凑方案：

```text
cell = 46
gap = 4
board = 4 * 46 + 5 * 4 = 204
x = 18
y = 36
```

推荐：

```text
board_x = 18
board_y = 40
cell = 46
gap = 4
board_w = 204
board_h = 204
```

但是底部提示栏会不够。因此建议：

```text
status_h = 30
footer_h = 24
board_x = 18
board_y = 34
cell = 46
gap = 4
board_h = 204
```

底部提示可覆盖在状态栏或暂停页里，不要强行保留大 footer。

## 12. 2048 局部刷新策略

先做简单可靠版本：

- 每次有效移动后刷新整个棋盘区域。
- 状态栏分数变化时刷新分数区域。
- 游戏结束时刷新状态提示。

这样已经比全屏刷新小很多，且实现简单。

第二阶段再优化：

- 只刷新变化格子。
- 新块生成只刷新新格子。
- 合并只刷新合并格子和来源格子。
- 分数只刷新数字区域。

不建议一开始做滑动动画。原因：

- 动画会引入更多 dirty 和状态复杂度。
- 先保证玩法完整更重要。

## 13. 新项目推荐目录结构

建议新项目目录：

```text
stm32f103_st7789_2048/
    Project/
    User/
        app_config.h
        main.c
        key_driver.c
        key_driver.h
        ui_event.c
        ui_event.h
        ui_types.h
        ui_dirty.c
        ui_dirty.h
        ui_renderer.c
        ui_renderer.h
        ui_draw.c
        ui_draw.h
        ui_font.c
        ui_font.h
        ui_page.c
        ui_page.h
        ui_feedback.c
        ui_feedback.h
        page_home.c
        page_home.h
        page_game.c
        page_game.h
        page_settings.c
        page_settings.h
        page_info.c
        page_info.h
        game_2048.c
        game_2048.h
        game_random.c
        game_random.h
        bsp_st7789.c
        bsp_st7789.h
        lcd_dma.c
        lcd_dma.h
    SYSTEM/
    Libraries/
    docs/
        README.md
        change-logs/
```

建议把游戏逻辑和页面绘制分开：

```text
game_2048.c      只负责规则、分数、胜负、棋盘变化
page_game.c      只负责按键事件、调用规则、绘制棋盘
```

这样后续调试会清楚很多。

## 14. 新项目阶段计划

### 阶段 0：新建工程骨架

目标：

- 从当前项目复制最小可用工程。
- 删除历史 GIF 和动画主线。
- 保留 LCD、DMA、按键、UI 基础。

验收：

- Keil 构建 0 error 0 warning。
- 上板显示 HOME 页面。
- INFO 页能显示统计。

### 阶段 1：2048 纯逻辑

目标：

- 实现 `game_2048.c/h`。
- 支持初始化、移动、合并、生成新块、分数、胜负判断。
- 不依赖 LCD。

建议接口：

```c
void Game2048_Init(uint32_t seed);
uint8_t Game2048_Move(Game2048_Direction dir);
uint8_t Game2048_IsWin(void);
uint8_t Game2048_IsOver(void);
uint32_t Game2048_GetScore(void);
uint8_t Game2048_GetCell(uint8_t row, uint8_t col);
```

验收：

- 用简单 host 测试或调试器验证移动规则。
- 合并规则符合 2048。
- 无动态内存。

### 阶段 2：GAME 页面静态绘制

目标：

- 新增 `page_game.c/h`。
- 画棋盘背景。
- 画 16 个格子。
- 显示数字和分数。

验收：

- 上板显示 4x4 棋盘。
- 初始两个块显示正确。
- 中文/ASCII 不重叠。

### 阶段 3：摇杆控制玩法

目标：

- UP/DOWN/LEFT/RIGHT 映射为四方向移动。
- 有效移动后生成新块。
- 分数更新。
- 无效移动不生成新块。

验收：

- 可以完整玩一局。
- 规则和手机 2048 一致。
- 按键连发不会造成脏区爆炸或卡屏。

### 阶段 4：胜负和重开

目标：

- 达到 2048 显示 WIN。
- 无可移动格子显示 GAME OVER。
- MID 重开。
- SET 暂停。

验收：

- 胜利和失败状态可靠。
- 重开后棋盘清空并生成两个新块。

### 阶段 5：局部刷新优化

目标：

- 初期整棋盘 dirty。
- 后续改为只刷新变化格子。
- 分数区域单独刷新。

验收：

- INFO 页 `KB` 明显低于全屏刷新。
- 快速操作无残影。
- `BUSY` / `DMA` 不持续异常增长。
- `DIRTY` overflow 不增长。

### 阶段 6：体验打磨

目标：

- 方块颜色表。
- 简单新块闪烁反馈。
- 合并反馈。
- 暂停菜单。
- 设置页主题切换。

验收：

- 游戏看起来完整。
- 操作反馈明确。
- 不牺牲稳定性。

## 15. 2048 推荐颜色

当前 UI 是 RGB565。

建议颜色表：

| 数值 | 颜色用途 |
| --- | --- |
| 空 | 深灰 |
| 2 | 浅灰 |
| 4 | 米白 |
| 8 | 橙 |
| 16 | 深橙 |
| 32 | 红橙 |
| 64 | 红 |
| 128 | 黄 |
| 256 | 金 |
| 512 | 青 |
| 1024 | 绿 |
| 2048 | 紫或白 |

注意：

- 不要做过多渐变。
- 字体要清楚。
- 小屏上数字可读性比视觉复杂度更重要。

## 16. 字体和数字显示建议

当前字体能力：

- 8x12 ASCII。
- 12x18 放大 ASCII。
- 16x16 中文子集。

2048 数字建议：

- 2、4、8、16 使用大号 ASCII。
- 128 以上仍可用大号 ASCII，但要居中。
- 1024、2048 可能较宽，需要缩小字距或用普通 ASCII。

建议增加一个 helper：

```c
UI_DrawTextCentered(rect, text, color)
```

不要一开始引入复杂字体。

## 17. 新项目风险点

### 17.1 按键连发太快

风险：

- 连续移动导致 dirty 排队过多。
- 方块状态变化过快，看不清。

建议：

- 游戏移动加 80 到 120 ms 节流。
- 保留按键消抖。
- 无效移动不刷新棋盘。

### 17.2 分数数字变长

风险：

- 分数超过区域宽度。

建议：

- 分数区域最多显示 5 到 6 位。
- 超过显示 `99999` 或滚动不做。

### 17.3 动画过早复杂化

风险：

- 滑动动画、合并动画、新块动画会显著增加状态复杂度。

建议：

- 第一版只做瞬时移动。
- 第二版只做新块闪烁。
- 第三版再考虑滑动动画。

### 17.4 颜色和文字重叠

风险：

- 4x4 格子空间有限。

建议：

- 先确定棋盘几何。
- 每个数字长度都实测。
- 不要在格子里放额外图标。

## 18. 新项目完成定义

最小可发布版本：

- 开机进入 HOME。
- 可以进入 GAME。
- 可以完整玩 2048。
- 四方向移动正确。
- 分数正确。
- 胜利和失败判断正确。
- MID 可重开。
- SET 可暂停或返回菜单。
- INFO 可看性能统计。
- Keil 构建 0 error 0 warning。
- 上板连续玩 10 分钟无花屏、无卡死。

理想版本：

- 有主题色。
- 有新块反馈。
- 有合并反馈。
- 有最好分数。
- 局部刷新只更新变化格子。
- INFO 中 `BUSY`、`DMA`、`DIRTY overflow` 在正常游玩中保持低水平。

## 19. 启动新项目时的第一步

建议下一次直接做：

1. 复制当前工程为新目录。
2. 改 README 和 Keil 工程名。
3. 删除历史 GIF 和 PLAYER 动画主线。
4. 保留 HOME / SETTINGS / INFO。
5. 新增 `game_2048.c/h`。
6. 新增 `page_game.c/h`。
7. HOME 第一项改为 `2048`。
8. 先实现无动画版本。

不要先做：

- 真 GIF。
- 大素材。
- 滑动动画。
- 复杂主题系统。
- 存档。

## 20. 总结

当前项目的价值不是“动画播放器”，而是已经打通了一个适合 STM32F103C8T6 的轻量显示和输入基础设施：

```text
按键输入
页面系统
脏矩形
窄条带 DMA
固定内存
基础控件
性能统计
```

下一个项目应该把这些基础设施用在真正有反馈、有规则、有目标的应用上。2048 是很合适的第一款游戏：规则简单、屏幕适配好、内存压力低、摇杆操作自然，也能继续验证局部刷新是否真的有价值。

