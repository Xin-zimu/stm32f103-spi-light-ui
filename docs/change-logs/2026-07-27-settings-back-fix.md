# 修复设置页无法返回

日期：2026-07-27

## 修改目标

修复进入 SETTINGS 页面后无法用摇杆左键返回的问题，使 `LEFT` 符合当前硬件语义中的“返回”行为。

## 修改前行为

SETTINGS 页面把 `UI_EVENT_LEFT` 当作“减小当前设置值”处理，因此用户进入设置页后按 LEFT 不会返回首页。虽然 RST 短按仍会产生全局 BACK，但实际摇杆 LFT 引脚在需求中也被定义为“向左/返回”，当前行为不符合预期。

## 修改后行为

SETTINGS 页面中：

- `LEFT`：返回 HOME。
- `RIGHT` / `MID`：修改当前选中的设置项。
- `UP` / `DOWN`：切换设置项。
- `RST` 短按：仍通过全局 BACK 返回 HOME。

底部提示改为 `RIGHT CHANGE LEFT BACK`。

## 逻辑变化范围

- `Page_Settings_ChangeValue()` 不再区分 LEFT/RIGHT 方向，只执行循环式修改。
- `Page_Settings_OnEvent()` 将 `UI_EVENT_LEFT` 改为调用 `UI_PageBack()`。
- SETTINGS 页脚提示同步更新。

## 涉及文件

- `User/page_settings.c`
- `docs/change-logs/2026-07-27-settings-back-fix.md`

## 接口与兼容性

- 对外接口不变。
- 页面管理器和按键驱动不变。
- 设置项仍为 RAM 临时状态，不涉及 Flash 保存。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，1 个路径。
- `test-new-function-comments.ps1`：通过。
- `git diff --check`：通过；仅有 Git 换行符提示，无空白错误。
- Keil isolated build：通过。
  - 工程：`Project\led.uvprojx`
  - 输出：`Output\codex-verify-20260727-021901-353`
  - 结果：0 Error(s), 0 Warning(s)
  - 大小：`Code=10072 RO-data=560 RW-data=8 ZI-data=2288`

## Git

- 分支：master
- 起始提交：7b25ad7a89a548afed57c965b7c1671cb729d1a1
- Commit：this commit
- 提交说明：Fix settings page back navigation
