# Silivid

**A video player and video enhancement tool for macOS on Apple Silicon, in development.**

[English](README.md) | [简体中文](README_CN.md)

> [!WARNING]
> Silivid has no usable version yet. There is no macOS app, release or installer to download.

Silivid is a fork of [Veyra](https://github.com/Likely7/Veyra-NRVideo), a real-time GPU video enhancement application for Windows. The goal is to bring a video player and enhancement pipeline to Apple Silicon Macs using Apple's native frameworks.

## What works today

Only the portable part of the code builds and runs on macOS, on the `macos/port` branch:

- Three static libraries with the code that does not depend on Windows.
- A build probe that links those libraries and checks the FFmpeg and Qt versions and the QML engine. It does not play video.
- A CTest suite of 24 entries: 20 test programs, the build probe and 3 cleanup steps.

## What does not exist yet on macOS

- An application or window.
- Video playback or audio.
- GPU processing, upscaling, denoising or frame generation.
- Capture, streaming, export or a virtual camera.

Most of the code is still inherited from Veyra and targets Windows, Direct3D 12 and NVIDIA or AMD GPUs. Documents marked as inherited describe that original application, not Silivid. No performance, image quality or hardware compatibility claims are made for macOS.

## Updates

Silivid has no working in-app updates, because there is no app yet. The planned update check only looks at `silivid-v<version>` releases of this repository, notifies you and opens the release page. It never downloads or installs anything. It has not run inside an app or read a real release.

## Build and test on macOS

You need an Apple Silicon Mac, Xcode and [Homebrew](https://brew.sh) installed in `/opt/homebrew`.

```sh
brew install cmake ninja pkgconf qt ffmpeg
git clone --branch macos/port https://github.com/davimf721/Silivid.git
cd Silivid
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

For a Release build, replace `debug` with `release`. Output goes to `build-macos/debug` or `build-macos/release`. The build does not produce an app.

Last verified on 2026-11-23 with macOS 27.2.1, Apple clang 21, CMake 4.5.1, Ninja 1.13.2, Qt 6.12.0 and FFmpeg 9.0.2. All 24 tests passed in both presets. Details and known blockers are in [`docs/macos/PHASE1.md`](docs/macos/PHASE1.md).

## Roadmap

| Phase | Scope | Status |
| --- | --- | --- |
| 1. Build | Native arm64 build of the portable code, with tests | Done |
| 2. Core | GPU abstraction and a Metal window showing decoded video | Not available yet |
| 3. Playback | VideoToolbox, audio, synchronization and color (SDR/HDR) | Not available yet |
| 4. Enhancement | MetalFX and denoising, measured on real hardware | Not available yet |
| 5. Capture | Capture devices and a virtual camera | Not available yet |
| 6. Distribution | Signed and notarized app for Apple Silicon | Not available yet |

**NVIDIA DLSS does not exist on macOS.** Silivid will not offer DLSS, and MetalFX should not be expected to match its models or results. The technology map is in [`docs/macos/PORTING.md`](docs/macos/PORTING.md).

## Contributing

GitHub Issues are disabled on this repository, and Silivid has no community or support channel yet. Please do not send Silivid problems to the original Veyra project.

If you change code, work from `macos/port`, keep platform code isolated, and run the build and tests above. Do not commit proprietary SDKs, DLLs, models or binaries. The project rules are in [`AGENTS.md`](AGENTS.md), and the documentation index is in [`docs/README.md`](docs/README.md).

## Credits

Silivid is based on **Veyra**, created by **Likely7** and its contributors.

- Original project and source code: <https://github.com/Likely7/Veyra-NRVideo>
- Original author: [X · @likely2002](https://x.com/likely2002) and [YouTube · @Likely7Ai](https://www.youtube.com/@Likely7Ai)
- Original Windows tutorials: [English on YouTube](https://www.youtube.com/watch?v=ZJ42jzNAbik) and [Chinese on Bilibili](https://www.bilibili.com/video/BV1KvsE6bEPw/)

Silivid is an independent fork and is not affiliated with or endorsed by the Veyra project. Veyra's community, donation and support channels belong to its author and are not Silivid channels.

## License

Silivid is a **modified version of Veyra**, distributed under the [GNU General Public License v3.0](LICENSE). This fork is based on Veyra commit `c6633121`. Changes made since then, starting on 2026-11-23, are recorded in this repository's Git history. Copyright in the original work remains with Veyra's authors and contributors.

The combined Chiaki streaming application inherited from Veyra is also subject to AGPL-3.0 and its OpenSSL exception. See [`licenses/remoteplay`](licenses/remoteplay). Other components keep their own licenses, listed in [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) and [`licenses/`](licenses).

The source code for this fork is in this repository. The source for the original project's releases is available from [Veyra](https://github.com/Likely7/Veyra-NRVideo), and the inherited [2.2.0 build and corresponding source notes](docs/BUILD_2.2.0.md) describe its latest release.
