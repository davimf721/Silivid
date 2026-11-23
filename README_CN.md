# Silivid

**面向 Apple Silicon macOS 的视频播放与画质增强工具，仍在开发中。**

[English](README.md) | 简体中文

> [!WARNING]
> Silivid 目前没有可用版本，也没有可下载的 macOS 应用或安装包。Silivid 的预发布版本（如有）仅包含源代码。

Silivid 是 [Veyra](https://github.com/Likely7/Veyra-NRVideo) 的分支。Veyra 是一款 Windows 实时 GPU 视频增强软件。本项目的目标是使用 Apple 原生框架，在 Apple Silicon Mac 上实现视频播放与增强处理链。

## 目前可用的部分

在 `macos/port` 分支上，macOS 目前只能构建并运行代码中的可移植部分：

- 四个静态库，包含不依赖 Windows 的代码和版本检查。
- 一个构建探针，用于链接这些库并检查 FFmpeg、Qt 版本和 QML 引擎。它不播放视频。
- 一套包含 24 个条目的 CTest 测试：20 个测试程序、构建探针和 3 个清理步骤。

## macOS 上尚不存在的功能

- 应用程序或窗口。
- 视频播放或音频。
- GPU 处理、超分辨率、降噪或补帧。
- 采集、串流、导出或虚拟摄像头。

大部分代码仍继承自 Veyra，其目标平台为 Windows、Direct3D 12 以及 NVIDIA 或 AMD 显卡。标记为继承的文档描述的是原软件，而不是 Silivid。现阶段不对 macOS 上的性能、画质或硬件兼容性作任何承诺。

## 更新

由于应用尚不存在，Silivid 目前没有可用的应用内更新。计划中的更新检查只读取本仓库 `silivid-v<版本>` 标签的发行版，发现新版本时提示并打开发行页面，绝不会下载或安装任何内容。它尚未在应用中运行，也尚未读取过真实的发行版。

## 在 macOS 上构建和测试

需要 Apple Silicon Mac、Xcode，以及安装在 `/opt/homebrew` 的 [Homebrew](https://brew.sh)。

```sh
brew install cmake ninja pkgconf qt ffmpeg
git clone --branch macos/port https://github.com/davimf721/Silivid.git
cd Silivid
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

构建 Release 时，将 `debug` 替换为 `release`。输出位于 `build-macos/debug` 或 `build-macos/release`。构建不会生成应用程序。

最近一次验证于 2026-11-23，环境为 macOS 27.2.1、Apple clang 21、CMake 4.5.1、Ninja 1.13.2、Qt 6.12.0 和 FFmpeg 9.0.2。两个预设的 24 项测试全部通过。详情和已知阻塞项见 [`docs/macos/PHASE1.md`](docs/macos/PHASE1.md)。

## 路线图

| 阶段 | 范围 | 状态 |
| --- | --- | --- |
| 1. 构建 | 可移植代码的原生 arm64 构建与测试 | 已完成 |
| 2. 核心 | GPU 抽象层，以及显示解码视频的 Metal 窗口 | 尚不可用 |
| 3. 播放 | VideoToolbox、音频、同步和色彩（SDR/HDR） | 尚不可用 |
| 4. 增强 | MetalFX 和降噪，并在真实硬件上测量 | 尚不可用 |
| 5. 采集 | 采集设备和虚拟摄像头 | 尚不可用 |
| 6. 分发 | 面向 Apple Silicon 的已签名并公证的应用 | 尚不可用 |

**macOS 上没有 NVIDIA DLSS。** Silivid 不会提供 DLSS，也不应期待 MetalFX 达到与其模型相同的效果。技术对照见 [`docs/macos/PORTING.md`](docs/macos/PORTING.md)。

## 参与贡献

本仓库已关闭 GitHub Issues，Silivid 目前也没有社区或支持渠道。请不要将 Silivid 的问题反馈给原 Veyra 项目。

修改代码时，请基于 `macos/port`，隔离平台相关代码，并运行上述构建和测试。不要提交专有 SDK、DLL、模型或二进制文件。项目规则见 [`AGENTS.md`](AGENTS.md)，文档索引见 [`docs/README.md`](docs/README.md)。

## 致谢

Silivid 基于 **Likely7** 及其贡献者创建的 **Veyra**。

- 原项目与源代码：<https://github.com/Likely7/Veyra-NRVideo>
- 原作者：[X · @likely2002](https://x.com/likely2002)、[YouTube · @Likely7Ai](https://www.youtube.com/@Likely7Ai)
- 原 Windows 版教程：[YouTube（英文）](https://www.youtube.com/watch?v=ZJ42jzNAbik)、[哔哩哔哩（中文）](https://www.bilibili.com/video/BV1KvsE6bEPw/)

Silivid 是独立分支，与 Veyra 项目没有隶属或背书关系。Veyra 的社区、赞助和支持渠道属于其作者，并非 Silivid 的渠道。

## 许可证

Silivid 是 **Veyra 的修改版本**，依据 [GNU 通用公共许可证第 3 版](LICENSE) 分发。本分支基于 Veyra 提交 `c6633121`，自 2026-11-23 起的修改记录在本仓库的 Git 历史中。原作品的版权仍归 Veyra 的作者和贡献者所有。

继承自 Veyra 的 Chiaki 串流组合程序同时适用 AGPL-3.0 及其 OpenSSL 例外，见 [`licenses/remoteplay`](licenses/remoteplay)。其他组件保留各自的许可证，见 [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) 和 [`licenses/`](licenses)。

本分支的源代码位于本仓库。原项目发行版的源代码可从 [Veyra](https://github.com/Likely7/Veyra-NRVideo) 获取，继承的 [2.2.0 构建与对应源码说明](docs/BUILD_2.2.0.md) 记录了其最新发行版。
