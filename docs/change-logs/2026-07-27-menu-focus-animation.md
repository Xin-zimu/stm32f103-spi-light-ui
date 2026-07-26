# 菜单焦点动画和交互反馈

日期：2026-07-27

## 修改目标

执行阶段七：在保持 GIF 关闭和脏矩形局部刷新的前提下，为 HOME 和 SETTINGS 增加轻量焦点滑动动画、设置项短按视觉反馈，并让页面具备周期任务调度能力。

## 修改前行为

HOME 和 SETTINGS 上下移动时只会立即切换行高亮，没有过渡反馈；设置项修改只刷新当前行但缺少 pressed 状态；页面管理器只能处理脏区绘制，不能让页面在无新按键时推进动画帧。

## 修改后行为

新增 160 ms Ease Out Cubic 焦点条动画。HOME 和 SETTINGS 上下移动时，选中行立即更新，同时左侧焦点条按约 16 ms 帧间隔滑到目标行。SETTINGS 修改值时当前行显示 60 ms pressed 反馈。页面切换时会清理旧反馈状态，避免不同页面同坐标控件误显示。

## 逻辑变化范围

- 新增 `ui_anim` 模块，提供固定状态的焦点动画、Ease Out Cubic 插值和动画脏区计算。
- 新增 `ui_feedback` 模块，提供 60 ms pressed 反馈状态和到期局部刷新。
- `UI_PageOps` 增加可选 `task(now)`，页面任务先推进反馈和当前页面动画，再处理脏区绘制。
- `ui_draw` 增加带 pressed 参数的中文菜单行绘制和左侧焦点条绘制。
- HOME 页面接入焦点动画，焦点变化时标记旧/新行并由页面 task 推进动画。
- SETTINGS 页面接入焦点动画，设置值变化时启动 pressed 反馈。
- `ui_dirty` 增加 union 脏区接口，供动画类刷新减少碎片。
- Keil 工程文件加入 `ui_anim.c/h` 和 `ui_feedback.c/h`。

## 涉及文件

- `User/ui_anim.c`
- `User/ui_anim.h`
- `User/ui_feedback.c`
- `User/ui_feedback.h`
- `User/ui_dirty.c`
- `User/ui_dirty.h`
- `User/ui_page.c`
- `User/ui_page.h`
- `User/ui_draw.c`
- `User/ui_draw.h`
- `User/page_home.c`
- `User/page_settings.c`
- `User/page_player.c`
- `User/page_info.c`
- `Project/led.uvprojx`

## 接口与兼容性

内部页面接口变化：`UI_PageOps` 增加 `task(now)` 函数指针，静态页面填 `0`。外部应用入口、按键映射、ST7789 引脚和 GIF 禁用状态不变。未使用动态内存，未引入全帧缓冲。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，15 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- Keil isolated build：0 error(s), 0 warning(s)。
- 程序体积：Code=11944, RO-data=1984, RW-data=16, ZI-data=2424。

## Git

- 分支：master
- 起始提交：d7cc15912b06695467a8dd15cde54742b9453234
- Commit：this commit
- 提交说明：Add menu focus animation
