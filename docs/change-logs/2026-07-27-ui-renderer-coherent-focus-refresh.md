# UI 条带渲染一致性刷新修复

日期：2026-07-27

## 修改目标

继续修复快速上下移动后的历史黄色残留和蓝色焦点条断裂问题。上一轮已扩大焦点 dirty 范围并加快条带提交，本次进一步保证条带刷新期间的动画状态一致，并避免旧行清理被新 dirty 长时间压住。

## 修改前行为

`UI_PageTask()` 先推进页面动画，再调用 `UI_RendererTask()`。当一个行级 dirty 被拆成多个 4 像素条带发送时，蓝色焦点动画可能在两个条带之间继续前进，导致同一个刷新区域的不同横向带使用了不同的焦点 Y 坐标，表现为蓝条断裂。

`UI_DirtyPop()` 使用后进先出方式取 dirty。快速上下移动会不断加入新的旧行、新行和动画 dirty，较早加入的旧行清理可能长期排在后面，屏幕上容易留下历史黄色选中背景片段。

## 修改后行为

`UI_PageTask()` 先服务 renderer。当 renderer 仍有 dirty 或 DMA 传输在进行时，页面动画任务暂不推进，焦点 Y 坐标保持不变，保证当前 dirty 的所有条带使用同一动画状态绘制。renderer 空闲后再推进下一帧动画。

`UI_DirtyPop()` 改为先进先出。旧行清理按产生顺序执行，不再被快速新按键产生的 dirty 反复插队。

## 逻辑变化范围

- `UI_PageTask()` 调整执行顺序：先 renderer，renderer 忙时返回，空闲时才推进 page-local animation task。
- `UI_DirtyPop()` 从 LIFO 改为 FIFO，最多移动 8 个 dirty 项，保持固定数组和无动态内存设计。
- 未修改 LCD DMA、ST7789、页面绘制函数和公开 UI 事件接口。

## 涉及文件

- `User/ui_page.c`
- `User/ui_dirty.c`

## 接口与兼容性

无公开接口变化。`UI_DirtyPop()` 行为从后进先出改为先进先出，但函数签名和调用方式保持不变。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1 -Paths User\ui_dirty.c, User\ui_page.c`：通过。
- `test-new-function-comments.ps1 -Paths User\ui_dirty.c, User\ui_page.c`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=11068, RO-data=1984, RW-data=40, ZI-data=6336。
- 构建日志：`Output\codex-verify-20260727-174501-764\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：9b9c527ff5437b55c45dcad3b44af9a4cd4e637d
- Commit：this commit
- 提交说明：Make UI strip refresh coherent
