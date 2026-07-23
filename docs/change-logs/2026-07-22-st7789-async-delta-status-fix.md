# ST7789 异步差分状态修复

日期：2026-07-22

## 修改目标

修复代码审查发现的两个异步差分刷新边界问题：

- DMA TE 或 delta 数据非法时，不能被上层当作一帧正常完成。
- 0 字节 delta 应表示“该 transition 没有像素变化”，不能导致动画卡住。

## 修改前行为

- `ST7789_AnimDeltaTask()` 只返回 `0/1`，其中 `0` 同时表示正常完成、空闲、DMA 错误或 delta 数据非法。
- `App_ST7789_AnimTask()` 只要看到 `ST7789_AnimDeltaTask() == 0U` 就推进帧序号；如果 DMA 或数据异常，后续差分会叠加到不确定画面上。
- `ST7789_AnimDeltaStart()` 拒绝 `delta_size == 0U`，未来素材中出现完全相同的相邻帧时会一直启动失败，帧序号无法推进。

## 修改后行为

- 新增 `ST7789_AnimDeltaResult`，把异步任务结果明确分为 `BUSY`、`DONE`、`ERROR`。
- `ST7789_AnimDeltaTask()` 只有正常完成或空闲时返回 `DONE`，DMA TE 或 delta 越界/截断时返回 `ERROR`。
- `ST7789_AnimDeltaStart()` 接受 0 字节 delta；下一次 task 会直接返回 `DONE`，上层正常推进到下一帧。
- `App_ST7789_AnimTask()` 只有收到 `DONE` 才推进 transition；收到 `ERROR` 时重画完整第一帧并重置动画调度，重新建立差分基准。

## 逻辑变化范围

- `User\bsp_st7789.h`：新增异步差分任务结果枚举，并更新 `ST7789_AnimDeltaTask()` 返回类型。
- `User\bsp_st7789.c`：更新异步差分任务返回语义；允许 0 长度 delta；保持原有 DMA 恢复流程。
- `User\app_st7789_anim.c`：新增首帧重画/调度重置 helper 和 delta 结果处理 helper；错误时不推进差分帧。

## 涉及文件

- `User\bsp_st7789.c`
- `User\bsp_st7789.h`
- `User\app_st7789_anim.c`
- `docs\change-logs\2026-07-22-st7789-async-delta-status-fix.md`

## 接口与兼容性

- `ST7789_AnimDeltaTask()` 的返回类型从 `uint8_t` 变为 `ST7789_AnimDeltaResult`。
- 调用方应根据 `BUSY/DONE/ERROR` 分支处理；不能再把 `0` 直接当作完成。
- `ST7789_AnimDeltaStart()` 保持 `uint8_t` 接口，但现在允许 `delta_size == 0U`。
- 其他同步显示接口和硬件映射不变。

## 编码与注释规范

- C/H：GB2312 兼容代码页 936，无 BOM。
- 文档及脚本：UTF-8，无 BOM。
- 函数前详细注释，函数内仅保留关键注释。
- 宏、枚举和结构字段使用对齐的行尾 // 注释。

## 验证结果

- `git diff --check`：通过。
- `test-stm32-text-policy.ps1`：4 个文件通过。
- `test-new-function-comments.ps1`：通过。
- Keil isolated build：`0 error(s), 0 warning(s)`。
- 程序体积：`Code=8190 RO-data=56746 RW-data=4 ZI-data=2268`。

## Git

- 分支：codex/usart1-tx-dma
- 起始提交：ac55799af8fbecc44c2137fac5b265c98fa5d358
- Commit：this commit
- 提交说明：Fix ST7789 async delta completion states
