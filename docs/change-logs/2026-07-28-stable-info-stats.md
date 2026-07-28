# 稳定 INFO 统计数字显示

日期：2026-07-28

## 修改目标

修复 INFO 页数字显示异常问题。确保 renderer 分条带绘制 INFO 页面时，同一个数字的所有横向切片来自同一份统计快照，而不是在每个 strip draw 中读取正在变化的实时计数。

## 修改前行为

`Page_Info_Draw()` 每次被 renderer 调用都会读取 `UI_RendererStats` 和 `UI_DirtyStats` 并重新格式化字符串。INFO 页面一个文本行会被 4 像素高条带分多次绘制，renderer 统计在绘制过程中持续变化，因此同一个数字的不同横向切片可能来自不同数值，上板看起来像数字乱码或非正常数字。

## 修改后行为

INFO 进入页面时调用 `Page_Info_CaptureStats()` 抓取一次稳定快照，之后整页所有条带都使用缓存字符串绘制。OK/RIGHT 会重新抓取快照并请求整页刷新。`Page_Info_FormatU32()` 在计数超过显示宽度时改为显示全 9 饱和值，避免出现误导性的截断数字。

## 逻辑变化范围

- `page_info.c` 新增静态文本缓存和 `Page_Info_CaptureStats()`。
- `Page_Info_OnEnter()` 抓取一次统计快照。
- `Page_Info_OnEvent()` 支持 OK/RIGHT 手动刷新统计快照。
- `Page_Info_Draw()` 不再读取实时统计，只绘制缓存字符串。
- `Page_Info_FormatU32()` 超出显示能力时输出全 9 饱和值。
- `README.md` 更新 INFO 快照显示说明和上板观察方式。

## 涉及文件

- `User/page_info.c`
- `README.md`
- `docs/change-logs/2026-07-28-stable-info-stats.md`

## 接口与兼容性

无公开接口变化。INFO 页面新增 OK/RIGHT 刷新快照行为；LEFT 返回、SET 进入设置等全局按键行为不变。不增加动态内存。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1 -Paths User\page_info.c, README.md, docs\change-logs\2026-07-28-stable-info-stats.md`：通过。
- `test-new-function-comments.ps1 -Paths User\page_info.c`：通过。
- `git diff --check`：通过。
- Keil 独立编译 `Project\led.uvprojx`：0 error(s), 0 warning(s)。
- 程序尺寸：Code=13500, RO-data=1988, RW-data=40, ZI-data=6448。
- 构建日志：`Output\codex-verify-20260728-193816-074\led.build_log.htm`。

## Git

- 分支：master
- 起始提交：f8046699272fafe92f344c84d14269e848231385
- Commit：this commit
- 提交说明：Stabilize INFO stats display
