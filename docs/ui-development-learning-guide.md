# STM32 SPI 轻量 UI 系统学习文档

日期：2026-07-27  
项目：STM32F103C8T6 + ST7789 SPI2 + 5D 摇杆轻量 UI  
当前状态：GIF 暂时移除，UI-first 路线已完成按键、页面框架、脏矩形、大字号中文、焦点动画和焦点条抽动修复。

## 1. 总体背景

这个项目最初已经有 ST7789 和 GIF 播放能力，但要在 STM32F103C8T6 这类 64 KB Flash、20 KB RAM 的芯片上继续叠加 UI，就不能直接使用 LVGL，也不能使用整屏 framebuffer。

核心约束：

- 不使用动态内存。
- 不使用全帧缓冲。
- UI 任务不能阻塞主循环。
- 按键采用定时扫描，不用外部中断。
- ST7789 继续使用 SPI2。
- 5D 摇杆本质是 7 个共地数字按键。
- GIF 数据很占 Flash，所以 UI 开发阶段先临时移除 GIF。

当前最终架构：

```text
PA0..PA6 5D 摇杆
    ↓
key_driver：扫描、消抖、长按、连发
    ↓
ui_event：物理按键转 UI 事件，固定环形队列
    ↓
app_ui：每轮处理有限数量事件
    ↓
ui_page：页面路由、全局返回/主页/复位、页面 task、脏区绘制
    ↓
page_home / page_settings / page_player / page_info
    ↓
ui_draw：裁剪绘制、中文/ASCII 字体、菜单行、状态栏、页脚
    ↓
ui_dirty：脏矩形队列
    ↓
ST7789_FillRect / ST7789_Clear
```

## 2. 阶段一：接入 5D 摇杆和最小 UI

### 阶段目标

把 5D 摇杆接入 UI 系统，建立最小输入链路。

硬件接线：

| 功能 | STM32 引脚 | 配置 |
| --- | --- | --- |
| UP | PA0 | 输入上拉 |
| DOWN | PA1 | 输入上拉 |
| LEFT | PA2 | 输入上拉 |
| RIGHT | PA3 | 输入上拉 |
| MID | PA4 | 输入上拉 |
| SET | PA5 | 输入上拉 |
| RST | PA6 | 输入上拉 |
| COM | GND | 共地 |

按键有效电平：

```c
pressed = HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET;
```

这个项目用的是 StdPeriph 工程，不是 CubeMX HAL 工程，但硬件逻辑一样：内部上拉，按下拉低。

### 功能模块

#### `key_driver`

职责：

- 初始化 PA0..PA6 为输入上拉。
- 每 10 ms 扫描一次。
- 20 ms 软件消抖。
- 方向键支持 repeat。
- RST 600 ms 产生 HOME。
- RST 2 秒产生 SYSTEM_RESET。

关键思想：

机械按键会抖动，如果用 EXTI，中断会在几毫秒内多次触发。UI 需要短按、长按、连发、组合处理，定时扫描比中断更可控。

典型逻辑：

```c
if (raw_pressed != stable_pressed)
{
    if ((now - last_change_ms) >= KEY_DEBOUNCE_MS)
    {
        stable_pressed = raw_pressed;
        if (stable_pressed)
        {
            push_press_event();
        }
    }
}
```

#### `ui_event`

职责：

- 把物理按键事件转换为 UI 事件。
- 使用固定大小环形队列，避免动态内存。

映射关系：

```text
UP    -> UI_EVENT_UP
DOWN  -> UI_EVENT_DOWN
LEFT  -> UI_EVENT_LEFT
RIGHT -> UI_EVENT_RIGHT
MID   -> UI_EVENT_OK
SET   -> UI_EVENT_SETTINGS
RST 短按 -> UI_EVENT_BACK
RST 长按 -> UI_EVENT_HOME
RST 2秒 -> UI_EVENT_SYSTEM_RESET
```

关键代码结构：

```c
typedef struct
{
    UI_Event items[UI_EVENT_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} UI_EventQueue;
```

### 当时的问题

最初 UI 很弱，因为为了保留 GIF 数据，Flash 几乎被 GIF 帧占满。构建结果中 `RO-data` 接近 56 KB，留给 UI 的空间很少。

问题表现：

- 首页几乎只是矩形占位。
- 没有像样字体。
- 设置/信息页无法展开。
- UI 和 GIF 强耦合。

### 分析思路

判断是不是 RAM 小：

- RAM 使用量并不高。
- 真正压力在 Flash，尤其是 GIF 帧数据。
- UI 本身代码几 KB，中文字库和控件也只需要几 KB。

结论：不是“UI 做不了”，而是“GIF 数据占用了 UI 迭代空间”。

### 解决方法

先临时移除 GIF 构建，保留源码和数据文件，不删除。

处理方式：

- 从 Keil 编译列表移除 `anim_frames.c/.h`。
- 从 Keil 编译列表移除 `app_st7789_anim.c/.h`。
- `main.c` 不再直接初始化 GIF 播放。
- PLAYER 页显示 `GIF DISABLED` 占位。

阶段结果：

```text
Code=9048
RO-data=472
RW-data=8
ZI-data=2168
```

Flash 立即释放，UI 可以继续发展。

## 3. 阶段二：模块化 UI 框架

### 阶段目标

把集中在 `app_ui.c` 的页面、绘制、事件分发拆开，避免后续变成单文件堆逻辑。

### 功能模块

#### `ui_types`

职责：

- 屏幕尺寸。
- 行高、边距、状态栏高度。
- UI 颜色。
- 页面 ID。
- 通用矩形结构。

关键结构：

```c
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} UI_Rect;
```

页面枚举：

```c
typedef enum
{
    UI_PAGE_HOME = 0,
    UI_PAGE_PLAYER,
    UI_PAGE_SETTINGS,
    UI_PAGE_INFO,
    UI_PAGE_COUNT
} UI_PageId;
```

#### `ui_draw`

职责：

- 封装 ST7789 基础绘制。
- 提供状态栏、页脚、菜单行、进度条、开关等控件。

早期接口：

```c
void UI_DrawClear(uint16_t color);
void UI_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void UI_DrawText(int16_t x, int16_t y, const char *text, uint16_t color);
void UI_DrawMenuRow(...);
```

#### `ui_page`

职责：

- 当前页面管理。
- 页面跳转。
- 全局事件优先级。
- 页面绘制调度。

页面接口：

```c
typedef struct
{
    void (*on_enter)(void);
    void (*on_event)(const UI_Event *event);
    void (*draw)(const UI_Rect *clip);
} UI_PageOps;
```

后续阶段加入：

```c
void (*task)(uint32_t now);
```

### 当时的问题

UI-first 版本虽然能显示页面，但结构上有问题：

- 页面状态、绘制、事件处理混在 `app_ui.c`。
- 后续加设置页、信息页、动画时会越来越难维护。
- 控件没有复用边界。

### 分析思路

如果继续在 `app_ui.c` 里堆功能，会出现三个后果：

- 修改一个页面容易影响其它页面。
- 脏矩形无法优雅落地，因为每个页面都需要知道自己的局部刷新区域。
- 动画和页面状态没有统一调度入口。

### 解决方法

把 UI 拆成四层：

```text
app_ui：只负责取事件和调用 UI_PageTask
ui_page：只负责页面路由和全局事件
page_xxx：只负责本页面状态和事件
ui_draw：只负责控件绘制
```

页面注册表：

```c
static const UI_PageOps * const UI_PAGES[UI_PAGE_COUNT] =
{
    &PAGE_HOME_OPS,
    &PAGE_PLAYER_OPS,
    &PAGE_SETTINGS_OPS,
    &PAGE_INFO_OPS
};
```

事件分发：

```c
switch (event->type)
{
    case UI_EVENT_SYSTEM_RESET:
        NVIC_SystemReset();
        break;

    case UI_EVENT_HOME:
        UI_PageHome();
        break;

    case UI_EVENT_SETTINGS:
        UI_PageGoto(UI_PAGE_SETTINGS);
        break;

    case UI_EVENT_BACK:
        UI_PageBack();
        break;

    default:
        UI_PAGES[g_ui_current_page]->on_event(event);
        break;
}
```

## 4. 阶段三：修复 SETTINGS 无法返回

### 问题表现

用户进入 SETTINGS 后，按 LEFT 出不来。

### 发现问题

通过实机操作发现：

```text
HOME -> SETTINGS
按 LEFT
页面仍停留在 SETTINGS
```

这说明硬件输入链路没有完全坏，因为 UP/DOWN/MID 仍能操作；问题大概率在 SETTINGS 页面事件处理。

### 分析问题

需求中 LEFT 的语义是：

```text
LEFT：向左 / 返回
```

但 SETTINGS 页面把 LEFT 当作“修改值”方向使用。这样就和全局返回语义冲突。

错误设计：

```text
LEFT/RIGHT 都用于改值
```

正确设计：

```text
LEFT：返回
RIGHT/MID：修改当前项
```

### 解决方法

修改 `page_settings.c`：

```c
else if (event->type == UI_EVENT_LEFT)
{
    UI_PageBack();
}
else if ((event->type == UI_EVENT_RIGHT) || (event->type == UI_EVENT_OK))
{
    Page_Settings_ChangeValue(event->timestamp);
}
```

### 经验

嵌入式 UI 的按键语义要稳定。尤其是返回键，不能在不同页面随意改变，否则用户会觉得“进去了出不来”。

## 5. 阶段四：脏矩形局部刷新

### 阶段目标

解决“每次刷新都是整个界面刷新”的割裂感。

### 问题表现

之前页面只有一个全局重画标志：

```c
static uint8_t g_ui_page_redraw_pending;
```

任何状态变化都做：

```c
UI_PageRequestRedraw();
```

然后页面绘制：

```c
UI_DrawClear(UI_COLOR_BG);
Page_Draw_All();
```

实际效果：

- 上下移动焦点，整页闪一下。
- 设置值变化，整页闪一下。
- 中文字以后会更慢。

### 分析问题

ST7789 没有 framebuffer 时，清屏和重画是直接写屏幕的。只要刷新区域大，肉眼就会看到：

```text
清掉 -> 空白/背景 -> 逐块补画
```

如果要让 UI 稳定，需要把刷新从“页面级”降到“控件区域级”。

### 解决方法

新增 `ui_dirty` 模块。

核心结构：

```c
#define UI_DIRTY_MAX_RECTS 8U

typedef struct
{
    UI_Rect rects[UI_DIRTY_MAX_RECTS];
    uint8_t count;
    uint8_t full_screen;
} UI_DirtyList;
```

核心接口：

```c
void UI_DirtyAdd(const UI_Rect *rect);
void UI_DirtyAddXYWH(int16_t x, int16_t y, int16_t w, int16_t h);
void UI_DirtyFullScreen(void);
uint8_t UI_DirtyPop(UI_Rect *rect);
```

页面层新增：

```c
void UI_PageInvalidate(const UI_Rect *rect);
void UI_PageInvalidateXYWH(int16_t x, int16_t y, int16_t w, int16_t h);
```

绘制层新增裁剪：

```c
void UI_DrawSetClip(const UI_Rect *clip);
void UI_DrawClearClip(uint16_t color);
```

`UI_PageTask()` 改为：

```c
if (UI_DirtyPop(&dirty) == 0U)
{
    return;
}

UI_DrawSetClip(&dirty);
UI_DrawClearClip(UI_COLOR_BG);
UI_PAGES[g_ui_current_page]->draw(&dirty);
UI_DrawSetClip(0);
```

### 页面如何使用

HOME 焦点移动：

```c
old_selected = g_home_selected;
g_home_selected = next_selected;
Page_Home_InvalidateRow(old_selected);
Page_Home_InvalidateRow(g_home_selected);
```

SETTINGS 修改值：

```c
Page_Settings_InvalidateRow(g_settings_selected);
```

PLAYER 状态变化：

```c
UI_PageInvalidateXYWH(30, 160, 180, 40);
```

### 优化点

脏区不是无限增加，而是：

- 超出屏幕先裁剪。
- 无效矩形丢弃。
- 相交或接近的矩形合并。
- 超过 8 个脏区就退化为全屏刷新。

这样保证：

- 不用动态内存。
- 不会无限堆积。
- 极端情况仍能正确显示。

## 6. 阶段五：大字号中文 UI

### 阶段目标

解决两个体验问题：

- 字体太小。
- 为什么不用中文。

### 问题表现

旧字体是 5x7 点阵，画在 8x12 cell 里：

```text
字很小
英文调试感强
不像正式 UI
```

### 分析问题

完整中文字库不适合直接放进 STM32F103C8T6：

- GB2312 全量字库太大。
- 每个 16x16 汉字 32 字节。
- 几千汉字会占用数百 KB。

但当前 UI 实际用字很少，比如：

```text
主页、动画播放器、设置、系统信息、亮度、主题、开、关、返回、确认、播放、暂停、停止、禁用中
```

所以应该用“中文子集字库”。

### 解决方法

#### `ui_font`

新增 16x16 中文字模表：

```c
typedef struct
{
    uint16_t code;
    uint8_t bitmap[32];
} UI_ChineseGlyph;
```

查字接口：

```c
const uint8_t *UI_FontGetChineseGlyph(uint16_t gb2312_code)
{
    uint8_t index;

    for (index = 0U;
         index < (uint8_t)(sizeof(UI_FONT_CN_TABLE) / sizeof(UI_FONT_CN_TABLE[0]));
         index++)
    {
        if (UI_FONT_CN_TABLE[index].code == gb2312_code)
        {
            return UI_FONT_CN_TABLE[index].bitmap;
        }
    }

    return 0;
}
```

中文字符串用 GB2312 字节转义，避免源码编码混乱：

```c
#define TEXT_HOME_TITLE "\xD6\xF7\xD2\xB3"  // 主页
```

#### `ui_draw`

新增中文绘制：

```c
void UI_DrawTextCN(int16_t x, int16_t y, const char *text, uint16_t color)
{
    const uint8_t *cursor = (const uint8_t *)text;

    while ((cursor != 0) && (*cursor != 0U))
    {
        if ((*cursor >= 0x80U) && (cursor[1] != 0U))
        {
            code = ((uint16_t)cursor[0] << 8) | cursor[1];
            UI_DrawChineseChar(x, y, code, color);
            x += UI_FONT_CN_WIDTH;
            cursor += 2;
        }
        else
        {
            UI_DrawCharLarge(x, y, (char)*cursor, color);
            x += UI_FONT_LARGE_WIDTH;
            cursor++;
        }
    }
}
```

新增大号 ASCII：

```c
static void UI_DrawCharLarge(int16_t x, int16_t y, char ch, uint16_t color)
{
    const uint8_t *glyph = UI_FontGetGlyph(ch);

    for (column = 0U; column < 5U; column++)
    {
        for (row = 0U; row < 7U; row++)
        {
            if ((glyph[column] & (1U << row)) != 0U)
            {
                UI_DrawRect(x + 1 + column * 2,
                            y + 2 + row * 2,
                            2,
                            2,
                            color);
            }
        }
    }
}
```

#### 页面布局

行高从 36 提升到 48：

```c
#define UI_ROW_H 48U
```

状态栏和底部提示也变大：

```c
#define UI_STATUS_H 32U
#define UI_FOOTER_H 28U
```

HOME 页面：

```c
UI_DrawStatusBar(TEXT_HOME_TITLE, UI_COLOR_ACCENT);
UI_DrawMenuRowCN(12, PAGE_HOME_ROW_Y0, 216, TEXT_PLAYER, ">", selected);
UI_DrawMenuRowCN(12, PAGE_HOME_ROW_Y0 + PAGE_HOME_ROW_STEP, 216, TEXT_SETTINGS, ">", selected);
UI_DrawMenuRowCN(12, PAGE_HOME_ROW_Y0 + PAGE_HOME_ROW_STEP * 2, 216, TEXT_INFO, ">", selected);
UI_DrawFooter(TEXT_FOOTER_HOME);
```

### 验证方法

写脚本检查页面中用到的中文是否都有字模：

```text
页面实际使用的 28 个中文字符均有字模，缺失 0 个。
```

构建结果：

```text
Code=10852
RO-data=1968
RW-data=16
ZI-data=2352
```

中文子集只增加约 1.4 KB 常量数据，可以接受。

## 7. 阶段六：菜单焦点动画和交互反馈

### 阶段目标

让 UI 不只是“静态可读”，而是有基本交互反馈。

目标功能：

- HOME / SETTINGS 上下移动时有焦点条滑动。
- SETTINGS 修改值时当前行有短暂 pressed 反馈。
- 页面拥有 `task(now)`，无按键时动画也能继续推进。

### 功能模块

#### `ui_anim`

焦点动画状态：

```c
typedef struct
{
    uint8_t active;
    int16_t from_y;
    int16_t to_y;
    int16_t current_y;
    int16_t last_y;
    uint32_t start_ms;
    uint32_t last_step_ms;
} UI_FocusAnim;
```

插值算法使用 Ease Out Cubic，避免浮点：

```c
static uint16_t UI_AnimEaseOutCubic(uint32_t elapsed_ms, uint16_t duration_ms)
{
    uint32_t t;
    uint32_t inv;

    if (elapsed_ms >= duration_ms)
    {
        return 1024U;
    }

    t = (elapsed_ms * 1024U) / duration_ms;
    inv = 1024U - t;

    return (uint16_t)(1024U - ((inv * inv * inv) >> 20));
}
```

动画帧推进：

```c
elapsed = now - anim->start_ms;
eased = UI_AnimEaseOutCubic(elapsed, UI_FOCUS_ANIM_MS);
distance = anim->to_y - anim->from_y;
next_y = anim->from_y + ((distance * eased) / 1024);
```

#### `ui_feedback`

职责：

- 保存 pressed 反馈矩形。
- 60 ms 后自动失效。
- 失效时重新标记该区域脏区。

核心结构：

```c
typedef struct
{
    uint8_t active;
    UI_Rect rect;
    uint32_t end_ms;
} UI_FeedbackState;
```

启动反馈：

```c
void UI_FeedbackPress(const UI_Rect *rect, uint32_t now)
{
    g_ui_feedback.active = 1U;
    g_ui_feedback.rect = *rect;
    g_ui_feedback.end_ms = now + UI_FEEDBACK_MS;
    UI_DirtyAdd(rect);
}
```

#### `ui_page`

页面接口增加 task：

```c
typedef struct
{
    void (*on_enter)(void);
    void (*on_event)(const UI_Event *event);
    void (*task)(uint32_t now);
    void (*draw)(const UI_Rect *clip);
} UI_PageOps;
```

页面任务：

```c
UI_FeedbackTask(now);

if (UI_PAGES[g_ui_current_page]->task != 0)
{
    UI_PAGES[g_ui_current_page]->task(now);
}

if (UI_DirtyPop(&dirty) != 0U)
{
    UI_DrawSetClip(&dirty);
    UI_DrawClearClip(UI_COLOR_BG);
    UI_PAGES[g_ui_current_page]->draw(&dirty);
    UI_DrawSetClip(0);
}
```

### 当时的问题

这个阶段加完后，用户反馈“效果很差，会有抽动”。随后确认是“焦点条抽动”。

这说明动画机制虽然有了，但刷新策略还没足够细。

## 8. 阶段七：焦点条抽动修复

### 问题表现

焦点条移动时能看到抽动，尤其是上下切换时。

### 发现问题的方法

首先缩小范围：

```text
中文 UI 静态显示正常
设置页和首页可操作
抽动主要发生在焦点条移动时
```

因此问题不在：

- 按键扫描。
- 页面跳转。
- 中文字库本身。

问题集中在：

- 焦点条动画脏区。
- 焦点条绘制策略。
- 连续按键时动画状态。

### 分析问题

查看 `UI_FocusAnimBuildDirty()` 后发现，焦点条每一帧返回的脏区是：

```c
dirty->x = 12;
dirty->w = 216;
dirty->h = old_y 到 new_y 的距离 + UI_ROW_H;
```

也就是说，视觉上只是 5 px 焦点条在动，但代码实际刷新的是：

```text
216 px 宽
跨越两行甚至多行的大块菜单区域
```

然后 `UI_PageTask()` 会：

```text
清掉这个大区域
调用页面 draw()
在 clip 内重画菜单行背景、中文文字、分隔线、焦点条
```

中文点阵是很多 1 px 小矩形，刷新大区域时速度慢，而且没有帧缓冲，肉眼就看到抽动。

第二个问题是连续按键。

旧逻辑：

```text
第 1 行 -> 第 2 行动画还没结束
用户又按 DOWN
新动画从第 2 行起点开始
```

但屏幕上的焦点条可能还在第 1 行和第 2 行之间，于是它会突然跳到第 2 行再继续动。

### 解决方法一：缩小焦点条脏区

旧脏区：

```c
dirty->x = 12;
dirty->y = top - 2;
dirty->w = 216;
dirty->h = (bottom - top) + UI_ROW_H + 4;
```

新脏区：

```c
dirty->x = 12;
dirty->y = top + 4;
dirty->w = 5;
dirty->h = (bottom - top) + UI_ROW_H - 8;
```

关键变化：

```text
宽度 216 px -> 5 px
```

这样每帧只刷新焦点条经过的窄条区域。

### 解决方法二：动画脏区不参与合并

脏矩形系统原本会合并相邻矩形，这对普通 UI 是好事，但对焦点条动画是坏事。

原因：

```text
焦点条 5px 窄区
如果和菜单行脏区合并
又会变回整行刷新
```

所以新增独立脏区接口：

```c
void UI_DirtyAddIsolated(const UI_Rect *rect)
{
    UI_Rect clipped;

    if (g_ui_dirty.full_screen != 0U)
    {
        return;
    }
    if (rect == 0)
    {
        return;
    }

    clipped = *rect;
    if (UI_DirtyClipRect(&clipped) == 0U)
    {
        return;
    }

    if (g_ui_dirty.count >= UI_DIRTY_MAX_RECTS)
    {
        UI_DirtyFullScreen();
        return;
    }

    g_ui_dirty.rects[g_ui_dirty.count] = clipped;
    g_ui_dirty.count++;
}
```

HOME 和 SETTINGS 的焦点条动画改用：

```c
UI_DirtyAddIsolated(dirty);
```

### 解决方法三：连续按键从当前位置继续动画

旧逻辑：

```c
anim->from_y = from_y;
anim->current_y = from_y;
```

新逻辑：

```c
start_y = (anim->active != 0U) ? anim->current_y : from_y;
anim->from_y = start_y;
anim->current_y = start_y;
```

效果：

```text
动画未结束又收到新方向键
焦点条从当前实际位置继续滑到新目标
不会回跳到上一行或目标行起点
```

### 结果

用户反馈“确实好很多了”。

构建结果：

```text
Code=12152
RO-data=1984
RW-data=16
ZI-data=2424
```

## 9. 当前各模块实现方法总结

### `key_driver`

功能：

- GPIOA PA0..PA6 输入上拉。
- 按下为低电平。
- 10 ms 扫描。
- 20 ms 消抖。
- 长按、连发、RST 复位事件。

优化点：

- 不用 EXTI，避免机械抖动导致多次中断。
- 方向键 repeat 和 RST 长按统一由扫描状态机处理。

### `ui_event`

功能：

- 物理按键转 UI 事件。
- 固定 16 项队列。
- 队列满时优先保留 BACK/HOME/SETTINGS/RESET 等关键事件。

优化点：

- 关键导航事件不能被方向键连发挤掉。
- 不使用 malloc。

### `ui_page`

功能：

- 页面路由。
- 全局事件优先级。
- 页面 task 调度。
- 脏区绘制调度。

优化点：

- 页面切换用全屏脏区。
- 页面内部变化用局部脏区。
- 页面切换时清理 feedback，避免 pressed 状态串页。

### `ui_dirty`

功能：

- 固定 8 个脏矩形。
- 裁剪屏幕边界。
- 普通脏区合并。
- 溢出转全屏。
- 动画条可用 isolated 脏区避免被合并。

优化点：

- 普通 UI 变化合并可以减少 ST7789 开窗次数。
- 动画 UI 变化不合并可以减少刷新面积。
- 两类需求要分开处理。

### `ui_draw`

功能：

- 矩形、边框、文字。
- 当前 clip 裁剪。
- 状态栏、页脚、菜单行、进度条、开关。
- 中文 GB2312 子集混排。
- 焦点条绘制。

关键优化：

- 所有绘制都经过 `UI_DrawRect()`，由它统一裁剪到当前 clip。
- 中文不做全字库，只做子集。
- 大号 ASCII 用 5x7 点阵 2 倍放大，不额外占用完整大字体表。

### `ui_font`

功能：

- 5x7 ASCII 字模。
- 16x16 中文子集字模。
- GB2312 code 查字。

优化点：

- 中文字符串使用 `\xD6\xF7` 形式存储，减少源码编码风险。
- 字库只收当前 UI 实际用字。

### `ui_anim`

功能：

- 焦点条动画。
- 160 ms Ease Out Cubic。
- 16 ms 左右推进一次。
- 连续按键时从当前位置继续动画。

优化点：

- 不用浮点。
- 不刷新整行。
- 只返回焦点条窄脏区。

### `ui_feedback`

功能：

- 60 ms pressed 反馈。
- 当前设置行短暂变色。
- 到期后自动标脏恢复。

优化点：

- feedback 是全局小状态，不为每个控件分配内存。
- 页面切换时清理。

## 10. 当前主要经验

### 经验一：先判断瓶颈是 Flash 还是 RAM

这个项目一开始效果差，不是因为 RAM 不能做 UI，而是 GIF 帧数据占 Flash。去掉 GIF 后 UI 空间立刻充足。

### 经验二：无 framebuffer 的屏幕动画必须小面积刷新

STM32F103 + ST7789 直接写屏幕时，动画不能靠“大区域清空再重画”。正确方向是：

```text
固定小区域
少量 FillRect
不重画文字
不重画整行背景
```

### 经验三：普通脏区和动画脏区策略不同

普通 UI：

```text
合并脏区，减少 LCD 开窗次数
```

动画：

```text
保持窄脏区，避免变成大面积重画
```

### 经验四：按键语义要稳定

LEFT 在设置页不能同时承担“返回”和“减小值”。当前设计：

```text
LEFT / RST：返回
RIGHT / MID：进入或修改
UP / DOWN：移动焦点
SET：打开设置
```

### 经验五：中文不是不能做，而是不能做全量字库

正确方式：

```text
当前 UI 用哪些字，就放哪些 16x16 字模
```

这样既能中文化，又不会吃掉 Flash。

## 11. 下一阶段建议

下一阶段可以做两条路线，建议先做路线 A。

### 路线 A：继续打磨 UI 质感

目标：

- 焦点条进一步减少残影。
- SETTINGS 的亮度进度条只刷新进度条区域。
- HOME 进入页面时增加短 pressed 反馈。
- 页脚文案更短，避免中文拥挤。

具体方法：

- 给菜单行拆分更细的区域：
  - 左侧焦点条区。
  - 文字区。
  - 右侧值区。
  - 进度条区。
- 设置值变化只刷新右侧值或进度条，不刷新整行。

### 路线 B：恢复 GIF 播放器组件

目标：

- 只在 PLAYER 页面恢复 GIF。
- HOME 和 SETTINGS 不受 GIF 影响。
- GIF 数据和 UI 资源一起评估 Flash。

关键原则：

- GIF 不再是全局主流程。
- GIF 是 PLAYER 页里的一个 widget。
- 离开 PLAYER 时停止 GIF task。
- 进入 PLAYER 时重置 GIF 状态。

## 12. 调试问题时推荐的流程

以后遇到“屏幕效果差”，不要直接猜代码。按这个顺序定位：

1. 确认问题范围：
   - 是所有页面？
   - 是某个控件？
   - 是按键后才出现？
   - 是动画中才出现？

2. 关掉复杂功能：
   - 关 GIF。
   - 关动画。
   - 只保留静态 UI。

3. 看脏区：
   - 打印或临时记录 `x/y/w/h`。
   - 重点看面积是不是异常大。

4. 看绘制耗时：
   - 绘制前拉高 GPIO。
   - 绘制后拉低 GPIO。
   - 用示波器或逻辑分析仪看高电平时间。

5. 再决定优化：
   - 如果脏区大，先缩小刷新区域。
   - 如果调用次数多，合并普通脏区。
   - 如果动画抖，动画脏区不要合并。
   - 如果文字慢，避免动画帧重画文字。

这套方法比直接堆功能更稳。

## 13. 关键基础概念在本项目中的使用

这一节从学习角度解释几个核心概念。你目前已经知道 DMA 和状态机，这两个概念已经覆盖了本项目的大部分核心逻辑；其它概念可以围绕它们理解。

### 13.1 DMA 在项目里做什么

DMA 可以理解成“硬件搬运工”。它负责把内存里的数据自动搬到外设寄存器，CPU 不需要一个字节一个字节地等着发送。

不用 DMA 时：

```text
CPU 写 1 个字节到 SPI
等待发送完成
CPU 再写下 1 个字节
继续等待
```

使用 DMA 后：

```text
CPU 准备好一段数据 buffer
启动 SPI DMA
DMA 自动把 buffer 里的数据搬到 SPI2->DR
SPI 自动发给 ST7789
CPU 回去继续扫描按键、处理 UI 状态
DMA 发送完成后再通过中断或状态位通知完成
```

在这个项目中，DMA 主要服务屏幕刷新：

- ST7789 发送命令和像素数据。
- 后续恢复 GIF 时发送图像块。
- 局部刷新时发送脏矩形区域内的数据。

UI 层做脏矩形优化，本质上是在减少 DMA/SPI 要搬运的数据量：

```text
整屏刷新：240 x 240 x 2 = 115200 字节
菜单行刷新：216 x 48 x 2 = 20736 字节
焦点条刷新：5 x 48 x 2 = 480 字节
```

所以焦点条从“整行区域刷新”优化成“只刷新 5px 窄条”，不只是 UI 逻辑变细了，也是 DMA/SPI 数据量大幅减少了。

### 13.2 DMA 有缓冲吗

DMA 本身通常不等于“大缓存”。它更像一个搬运控制器。

真正的缓冲区一般在 RAM 里，由程序定义：

```c
static uint8_t spi_tx_buf[128];
```

DMA 工作时的关系是：

```text
RAM buffer[]  ->  DMA  ->  SPI2->DR  ->  ST7789
```

所以更准确的说法是：

```text
DMA 使用我们提供的缓冲区。
DMA 自己不保存一整张屏幕。
```

启动 DMA 后，buffer 在 DMA 发送完成前不能随便改：

```c
Start_DMA(buffer);
buffer[0] = 0x00;   // 危险：DMA 可能还在读这个 buffer
```

否则屏幕收到的数据可能混乱。

常见两种缓冲方式：

#### 单缓冲

```text
只有 buffer A
DMA 正在发送 A 时，CPU 不能改 A
DMA 完成后，CPU 才能重新填 A
```

优点：

- 省 RAM。
- 实现简单。

缺点：

- CPU 和 DMA 容易互相等待。

#### 双缓冲

```text
DMA 正在发送 buffer A
CPU 同时准备 buffer B
A 发完后，DMA 切到 B
CPU 再准备 A
```

优点：

- 更流畅。
- 适合 GIF、图片块、条带刷新。

缺点：

- 多占一份 RAM。
- 状态管理更复杂。

本项目现在 UI 阶段没有使用全屏 framebuffer，而是靠小块区域刷新。后续如果恢复 GIF，比较合理的是使用“小条带双缓冲”，而不是整屏双缓冲。

### 13.3 状态机在项目里怎么用

状态机不是一个特定文件，而是一种写法：系统根据“当前状态”和“输入事件”决定下一步。

本项目里有很多状态机。

#### 按键状态机

按键不是读到低电平就立刻算一次按下，而是要经历：

```text
未按下
检测到低电平
等待消抖
确认按下
长按计时
连发
释放
```

RST 键又有额外状态：

```text
短按：返回
按住 600ms：回主页
按住 2s：软件复位
```

这就是典型状态机。

#### 页面状态机

当前页面就是状态：

```text
HOME
PLAYER
SETTINGS
INFO
```

事件决定状态转换：

```text
HOME + OK      -> PLAYER / SETTINGS / INFO
SETTINGS + LEFT -> HOME
任意页面 + SET  -> SETTINGS
任意页面 + RST长按 -> HOME
```

代码里由 `ui_page` 管理：

```c
static UI_PageId g_ui_current_page = UI_PAGE_HOME;
```

#### 页面内部状态机

HOME 页面内部状态：

```c
static uint8_t g_home_selected;
```

SETTINGS 页面内部状态：

```c
static uint8_t g_settings_selected;
static uint8_t g_settings_animation;
static uint8_t g_settings_brightness;
static uint8_t g_settings_theme;
```

PLAYER 页面内部状态：

```c
typedef enum
{
    PLAYER_STATE_STOPPED = 0,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_PAUSED
} PlayerState;
```

#### 动画状态机

焦点条动画也是状态机：

```c
typedef struct
{
    uint8_t active;
    int16_t from_y;
    int16_t to_y;
    int16_t current_y;
    uint32_t start_ms;
} UI_FocusAnim;
```

状态变化：

```text
未动画
收到 UP/DOWN
动画中
到达目标
停止
```

### 13.4 事件队列是什么

事件队列可以理解成“按键消息排队”。

按键驱动不直接操作页面，而是先产生事件：

```text
KEY_ID_DOWN + KEY_EVENT_PRESS
    ↓
UI_EVENT_DOWN
    ↓
放入 UI 事件队列
```

然后 UI 主循环再取出来处理：

```c
for (processed = 0U; processed < UI_MAX_EVENTS_PER_TASK; processed++)
{
    if (UI_EventPop(&event) == 0U)
    {
        break;
    }
    UI_PageDispatchEvent(&event);
}
```

为什么要用队列：

- 按键扫描和页面绘制解耦。
- UI 绘制慢一点时，按键事件不会立刻丢。
- 每轮最多处理固定数量事件，避免按键连发把屏幕刷新饿死。

队列结构：

```c
typedef struct
{
    UI_Event items[UI_EVENT_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} UI_EventQueue;
```

这也是“不使用动态内存”的体现：队列长度固定为 16。

### 13.5 脏矩形是什么

脏矩形就是“屏幕上发生变化、需要重画的区域”。

不使用脏矩形时：

```text
按一下 DOWN
清全屏
重画状态栏
重画所有菜单行
重画页脚
```

使用脏矩形后：

```text
按一下 DOWN
只标记旧选中行和新选中行
只刷新这两块区域
```

焦点条修复后更细：

```text
焦点条移动
只刷新 5px 宽的焦点条路径
```

脏矩形模块固定保存 8 个区域：

```c
typedef struct
{
    UI_Rect rects[UI_DIRTY_MAX_RECTS];
    uint8_t count;
    uint8_t full_screen;
} UI_DirtyList;
```

普通脏区会尝试合并：

```text
两个区域相交或很近
合成一个更大的区域
减少 ST7789 设置窗口次数
```

动画脏区不合并：

```text
焦点条必须保持 5px 窄区域
否则合并后又变成整行刷新
```

这就是为什么后来加了：

```c
void UI_DirtyAddIsolated(const UI_Rect *rect);
```

### 13.6 裁剪绘制是什么

脏矩形告诉系统“哪里需要刷新”，裁剪绘制负责保证“只往这个区域写屏幕”。

例如页面 draw 里仍然写：

```c
UI_DrawStatusBar(...);
UI_DrawMenuRowCNEx(...);
UI_DrawFooter(...);
```

看起来像整页都画了一遍，但底层 `UI_DrawRect()` 会检查当前 clip：

```text
这个矩形和当前脏区不相交 -> 不画
这个矩形和当前脏区相交 -> 只画相交部分
```

关键接口：

```c
void UI_DrawSetClip(const UI_Rect *clip);
void UI_DrawClearClip(uint16_t color);
```

页面任务中这样用：

```c
UI_DrawSetClip(&dirty);
UI_DrawClearClip(UI_COLOR_BG);
UI_PAGES[g_ui_current_page]->draw(&dirty);
UI_DrawSetClip(0);
```

可以这样理解：

```text
脏矩形：决定这次要修哪块墙
裁剪绘制：保证刷子不会刷到墙外
```

### 13.7 不使用动态内存是什么意思

不使用动态内存，就是运行时不调用：

```c
malloc();
free();
calloc();
realloc();
```

也不在运行中临时申请不确定大小的 RAM。

本项目采用固定内存：

```c
static UI_Event items[16];
static UI_Rect rects[8];
static UI_FocusAnim g_home_focus_anim;
```

这样做的原因：

- STM32F103C8T6 RAM 很小。
- 动态内存容易碎片化。
- `malloc()` 失败后 UI 很难优雅恢复。
- 固定内存更容易估算最坏情况。

嵌入式 UI 更看重稳定性，而不是运行时灵活申请。

### 13.8 不使用全帧缓冲是什么意思

全帧缓冲就是在 RAM 里保存整张屏幕图像。

ST7789 是 240x240，RGB565 每个像素 2 字节：

```text
240 x 240 x 2 = 115200 字节
```

也就是约 112.5 KB。

STM32F103C8T6 RAM 只有约 20 KB，所以不能这样做：

```c
uint16_t framebuffer[240][240];
```

这个数组本身就已经超过芯片 RAM。

因此本项目采用：

```text
哪里变化，就直接往 ST7789 写哪里
```

这也是为什么脏矩形、裁剪绘制、小面积动画非常重要。

### 13.9 这些概念之间的关系

可以用一条链路理解：

```text
状态机决定 UI 当前是什么状态
事件队列把按键变化排队交给状态机
状态变化产生脏矩形
裁剪绘制限制真正写屏区域
DMA/SPI 把这些小区域的数据发给 ST7789
```

也可以换成一句话：

```text
状态机决定“画什么”，脏矩形决定“画哪里”，裁剪保证“不多画”，DMA 负责“把数据发出去”。
```

这个项目目前最核心的优化，都是围绕这句话展开的。
