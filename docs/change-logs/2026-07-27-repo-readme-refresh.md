# 刷新仓库 README 和忽略规则

日期：2026-07-27

## 修改目标

审查并刷新新 GitHub 仓库的对外说明文件，让 README 与当前 UI-first 项目状态一致，并完善忽略规则，避免本地 GIF 素材和 Keil 输出产物被误提交。

## 修改前行为

README 仍描述旧的 ST7789 小猫 GIF 自动播放工程，包含 `anim_frames.c`、`app_st7789_anim.c` 和上电播放 GIF 的验收标准。当前实际主线已经切换为轻量 UI 项目，GIF 数据和播放模块暂不参与构建。`.gitignore` 未忽略本地 GIF 素材和多类 Keil 生成产物。

## 修改后行为

README 改为当前轻量 UI 项目入口，说明 5D 摇杆接线、UI-first 状态、页面结构、软件架构、关键实现、编译方法、按键语义和后续计划。旧 GIF 文档作为历史资料保留在 docs 中。`.gitignore` 增加 Keil 中间产物、hex/axf/map、GIF 素材等忽略规则。

## 逻辑变化范围

代码逻辑与原提交完全一致。本次只修改仓库说明和忽略规则。

## 涉及文件

- `README.md`
- `.gitignore`
- `docs/change-logs/2026-07-27-repo-readme-refresh.md`

## 接口与兼容性

固件接口、Keil 工程配置、硬件引脚、按键语义和构建输出均不变。`.gitignore` 新增 `*.gif` 忽略规则，历史 docs 下若需要放 GIF 可用 `!docs/**/*.gif` 例外。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `test-stm32-text-policy.ps1`：通过，3 个路径。
- `git diff --check`：通过，仅有本地 CRLF 转换提示。
- README 关键字审查：README 已无旧“上电播放小猫 GIF”项目描述；旧 GIF 内容仅保留在历史 docs/change-logs 文档中。
- Keil isolated build：0 error(s), 0 warning(s)。
- 程序体积：Code=12152, RO-data=1984, RW-data=16, ZI-data=2424。

## Git

- 分支：master
- 起始提交：4fdc2c694626c473665e001f1955ff1a8a8d513b
- Commit：this commit
- 提交说明：Refresh repository README
