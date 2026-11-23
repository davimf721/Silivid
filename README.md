# Silivid

**A video player and video enhancement tool for macOS on Apple Silicon, in development.**

[English](README.md) | [简体中文](README_CN.md)

> [!WARNING]
> Silivid has no usable version yet. There is no macOS app, release or installer to download.

Silivid is a fork of [Veyra](https://github.com/Likely7/Veyra-NRVideo), a real-time GPU video enhancement application for Windows. The goal is to bring a video player and enhancement pipeline to Apple Silicon Macs using Apple's native frameworks.

## Project status

This repository still contains the code inherited from Veyra, which targets Windows, Direct3D 12 and NVIDIA or AMD GPUs. Features described in the inherited documents under `docs/` belong to the original Windows application unless a document says otherwise. They should not be read as working Silivid features on macOS.

No performance, image quality or hardware compatibility claims are made for macOS at this stage.

## Goals

These are development goals, not finished features:

- **Metal** for GPU processing and video presentation.
- **MetalFX** for upscaling, and an evaluation of its frame interpolation for video.
- **VideoToolbox** for hardware video decoding and encoding.

**NVIDIA DLSS does not exist on macOS.** Silivid will not offer DLSS, and MetalFX should not be expected to match its models or results.

## Installation

There is nothing to install yet. The Windows downloads and installation steps from Veyra do not apply to macOS. macOS build instructions will be added once the port can be built and verified.

## Credits

Silivid is based on **Veyra**, created by **Likely7** and its contributors.

- Original project and source code: <https://github.com/Likely7/Veyra-NRVideo>
- Original author: [X · @likely2002](https://x.com/likely2002) and [YouTube · @Likely7Ai](https://www.youtube.com/@Likely7Ai)
- Original Windows tutorials: [English on YouTube](https://www.youtube.com/watch?v=ZJ42jzNAbik) and [Chinese on Bilibili](https://www.bilibili.com/video/BV1KvsE6bEPw/)

Silivid is an independent fork and is not affiliated with or endorsed by the Veyra project. Please do not send Silivid problems to the original project. Veyra's community, donation and support channels belong to its author and are not Silivid channels.

## License

Silivid is a **modified version of Veyra**, distributed under the [GNU General Public License v3.0](LICENSE). This fork is based on Veyra commit `c6633121`, and this documentation was modified on 2026-11-23. Copyright in the original work remains with Veyra's authors and contributors.

The combined Chiaki streaming application inherited from Veyra is also subject to AGPL-3.0 and its OpenSSL exception. See [`licenses/remoteplay`](licenses/remoteplay). Other components keep their own licenses, listed in [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) and [`licenses/`](licenses).

The source code for this fork is in this repository. The source for the original project's releases is available from [Veyra](https://github.com/Likely7/Veyra-NRVideo), and the inherited [2.2.0 build and corresponding source notes](docs/BUILD_2.2.0.md) describe its latest release.
