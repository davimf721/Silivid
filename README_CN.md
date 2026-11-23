# Silivid

**面向 Apple Silicon macOS 的视频播放与画质增强工具，仍在开发中。**

[English](README.md) | 简体中文

> [!WARNING]
> Silivid 目前没有可用版本，也没有可下载的 macOS 应用、发行版或安装包。

Silivid 是 [Veyra](https://github.com/Likely7/Veyra-NRVideo) 的分支。Veyra 是一款 Windows 实时 GPU 视频增强软件。本项目的目标是使用 Apple 原生框架，在 Apple Silicon Mac 上实现视频播放与增强处理链。

## 项目状态

本仓库目前仍包含继承自 Veyra 的代码，其目标平台为 Windows、Direct3D 12 以及 NVIDIA 或 AMD 显卡。除非文档另有说明，`docs/` 中继承的文档所描述的功能都属于原 Windows 软件，不代表 Silivid 已在 macOS 上实现这些功能。

现阶段不对 macOS 上的性能、画质或硬件兼容性作任何承诺。

## 开发目标

以下是开发目标，并非已完成的功能：

- **Metal**：用于 GPU 处理和视频呈现。
- **MetalFX**：用于超分辨率，并评估其帧插值是否适用于视频。
- **VideoToolbox**：用于硬件视频解码和编码。

**macOS 上没有 NVIDIA DLSS。** Silivid 不会提供 DLSS，也不应期待 MetalFX 达到与其模型相同的效果。

## 安装

目前没有可安装的内容。Veyra 的 Windows 下载和安装步骤不适用于 macOS。待移植版本可以构建并完成验证后，再补充 macOS 构建说明。

## 致谢

Silivid 基于 **Likely7** 及其贡献者创建的 **Veyra**。

- 原项目与源代码：<https://github.com/Likely7/Veyra-NRVideo>
- 原作者：[X · @likely2002](https://x.com/likely2002)、[YouTube · @Likely7Ai](https://www.youtube.com/@Likely7Ai)
- 原 Windows 版教程：[YouTube（英文）](https://www.youtube.com/watch?v=ZJ42jzNAbik)、[哔哩哔哩（中文）](https://www.bilibili.com/video/BV1KvsE6bEPw/)

Silivid 是独立分支，与 Veyra 项目没有隶属或背书关系。请不要将 Silivid 的问题反馈给原项目。Veyra 的社区、赞助和支持渠道属于其作者，并非 Silivid 的渠道。

## 许可证

Silivid 是 **Veyra 的修改版本**，依据 [GNU 通用公共许可证第 3 版](LICENSE) 分发。本分支基于 Veyra 提交 `c6633121`，本文档修改于 2026-11-23。原作品的版权仍归 Veyra 的作者和贡献者所有。

继承自 Veyra 的 Chiaki 串流组合程序同时适用 AGPL-3.0 及其 OpenSSL 例外，见 [`licenses/remoteplay`](licenses/remoteplay)。其他组件保留各自的许可证，见 [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) 和 [`licenses/`](licenses)。

本分支的源代码位于本仓库。原项目发行版的源代码可从 [Veyra](https://github.com/Likely7/Veyra-NRVideo) 获取，继承的 [2.2.0 构建与对应源码说明](docs/BUILD_2.2.0.md) 记录了其最新发行版。
