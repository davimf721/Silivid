# Silivid documentation

## Current

These documents describe Silivid as it is today.

- [Project overview, build, roadmap and contributing](../README.md) ([简体中文](../README_CN.md))
- [Phase 1 results: what builds, what is disabled and what blocks Phase 2](macos/PHASE1.md)
- [macOS porting plan and technology map](macos/PORTING.md)
- [Release notifier: how it works and what is still unverified](macos/UPDATES.md)
- [Project rules](../AGENTS.md)
- [License](../LICENSE), [third-party notices](../THIRD_PARTY_NOTICES.md) and [component licenses](../licenses)

Silivid has no usable app yet. On macOS, only the portable code, its tests and a build probe work.

## Inherited from Veyra

Everything below describes the original [Veyra](https://github.com/Likely7/Veyra-NRVideo) application for Windows. Its features, paths, builds and test results are not Silivid features on macOS. Words such as "current" or "unreleased" refer to the original project at the date of each document.

以下内容继承自原 Windows 版 Veyra，不代表 Silivid 在 macOS 上的功能。文中的“当前/未发布”均按原项目和原日期解释。

1. [当前项目状态](CURRENT_STATUS.md)：正式版、活动工作、暂停项、现场问题及验收边界。
2. [开发与验证](DEVELOPMENT.md)：唯一施工树、构建、回归、运行供体和路径规则。
3. [当前工作日志](WORKLOG.md)：正在进行的实际结果与续接信息。
4. [项目整理执行记录](PROJECT_MAINTENANCE_EXECUTION_2026-10-27.md)：本轮清理与结构整理。
5. [分支与工作树归档](BRANCH_ARCHIVE.md)：保留项、已退休分支tip与恢复方法。
6. [2.1.1用户更新说明](RELEASE_NOTES_2.1.1.md) 与 [对应源码/构建](BUILD_2.1.1.md)。最新发行版见 [2.2.0 更新说明](RELEASE_NOTES_2.2.0.md) 与 [对应源码/构建](BUILD_2.2.0.md)。
7. [界面代码与职责边界](ARCHITECTURE.md)：桥接模块与QML组件从哪里找。
8. [当前双重超分施工](DUAL_SR_PLAN_2026-10-28.md)：基础SR→降低AI结果→第二次SR；覆盖同日早期单层方案与4K限制。随后新增[超采样选项](SR_SUPERSAMPLE_2026-10-28.md)：单次RTX视频超分到更高尺寸后精确缩回。
9. [Veyra 原项目的支持渠道](RELEASE_SUPPORT.md)：社区、赞助和二维码均属于原作者，并非 Silivid 的渠道。

Original index notes / 原索引记录：

- 当前独立分支已完成续接收尾、未提交：[垂直同步等待上限3 ms](VSYNC_JITTER_WAIT_2026-11-02.md)。
- 最新并入本地main（未发布）：[11-01合并记录](MAIN_INTEGRATION_2026-11-01.md)、[补帧返工与VFG自动调控](FG_NO_LATENCY_GROWTH_2026-11-01.md)。
- 最近并入本地 main（未发布）：[补帧接近刷新率的间歇掉帧](FG_60HZ_PACING_2026-10-31.md)、[英伟达低延迟三档](NVIDIA_LOW_LATENCY_2026-10-28.md)、[RX 7000 NR](AMD_RDNA3_NR_PLAN_2026-10-29.md)、[屏幕捕获与采集音频](CAPTURE_FPS_AUDIO_REPORT_2026-10-27.md)。

历史方案、排查和发行过程集中在[历史索引](history/INDEX.md)。旧路径对照[PATH_MAP.json](history/PATH_MAP.json)；这些历史文件中的“当前/未合并/未发布”按原日期解释。
