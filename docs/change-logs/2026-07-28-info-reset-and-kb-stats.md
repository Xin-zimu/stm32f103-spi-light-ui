# INFO 清零统计和 KB 显示

日期：2026-07-28

## 修改目标

落实 INFO 页按键语义优化：OK/MID 用于清零 renderer/dirty 统计并显示清零快照，RIGHT 用于刷新当前快照但不清零；同时把累计发送字节改成 KB 显示，减少上板观察时的大数字误解。

## 修改前行为

INFO 页已经使用稳定快照避免分条带绘制时数字拼接错误，但 OK/MID 和 RIGHT 都只是刷新快照。由于刷新 INFO 页本身也会产生条带和字节统计，用户按键后看到数值继续增加，容易误以为按键在修改数值。BYTES 原始字节数增长较快，也不适合小屏直接阅读。

## 修改后行为

INFO 页按键语义改为：LEFT 返回，OK/MID 清零 renderer/dirty 统计并缓存清零后的显示，RIGHT 只刷新当前统计快照。字节统计改为 `KB` 行，按 1024 字节向上折算，保留饱和数字显示规则。页脚改为 `L BACK OK CLR R REF`，直接提示当前页面的三个操作。

## 逻辑变化范围

- `page_info.c` 新增 `Page_Info_FormatKB()`，用 KB 显示 `bytes_submitted`。
- `page_info.c` 新增 `Page_Info_ResetStats()`，按 OK/MID 时调用 `UI_RendererResetStats()` 和 `UI_DirtyResetStats()` 后缓存清零快照。
- `Page_Info_OnEvent()` 区分 OK/MID 清零与 RIGHT 刷新。
- INFO 页标签从 `BYTES` 调整为 `KB`，页脚调整为 ASCII 操作提示。
- `README.md` 更新 INFO 行说明和上板观察步骤。

## 涉及文件

- `User/page_info.c`
- `README.md`
- `docs/change-logs/2026-07-28-info-reset-and-kb-stats.md`

## 接口与兼容性

无新增公开接口。复用已有 `UI_RendererResetStats()` 和 `UI_DirtyResetStats()`。INFO 页面按键语义变化：OK/MID 从刷新快照改为清零统计，RIGHT 保留刷新快照。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1 -Paths User\page_info.c, README.md, docs\change-logs\2026-07-28-info-reset-and-kb-stats.md`：通过。
- `test-new-function-comments.ps1 -Paths User\page_info.c`：通过。
- `git diff --check`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=13584, RO-data=1988, RW-data=40, ZI-data=6448。
- 构建日志：`Output\codex-verify-20260728-194459-709\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：83850ac3f8080b4fe3f8290b5298762c5723f5c7
- Commit：this commit
- 提交说明：Add INFO reset and KB stats
