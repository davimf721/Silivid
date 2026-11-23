# Silivid: porte do Veyra para macOS (Apple Silicon)

Fork: `davimf721/Silivid`. Upstream: `Likely7/Veyra-NRVideo`. Licença: GPL-3.0.
Branch de trabalho: `macos/port`. A `main` fica limpa para acompanhar o upstream.

## Ambiente

- Alvo: macOS em arm64. Máquina de referência: Mac mini M4, 16 GB.
- Ferramentas: Xcode, CMake, Ninja, Qt 6 e FFmpeg (Homebrew), uv.
- Mantenha o repositório e os builds em APFS (`~/Developer`). Não use o SSD externo exFAT.

## Mapa de tecnologias

| Windows (upstream) | macOS (alvo) |
| --- | --- |
| Direct3D 12 / DXGI | Metal |
| Shaders HLSL | Metal Shading Language |
| NVIDIA DLSS / NGX | MetalFX (upscaling), sem paridade de modelo |
| Geração de quadros DLSS/FSR | MetalFX Frame Interpolation, a validar para vídeo |
| Media Foundation / DirectShow | AVFoundation, VideoToolbox |
| Placa de captura | AVFoundation / CoreMediaIO |
| Câmera virtual DirectShow | CoreMediaIO Camera Extension |
| WASAPI | Core Audio |
| Qt/QML | Qt/QML (reaproveitar) |

## Fases

1. Build: CMake configura e compila em macOS com os módulos Windows desligados. Concluída para o código portável, veja [PHASE1.md](PHASE1.md).
2. Núcleo: abstração de GPU e janela Metal exibindo vídeo decodificado.
3. Reprodução: VideoToolbox, áudio, sincronização e cor (SDR/HDR).
4. Aprimoramento: MetalFX e redução de ruído, com medição no M4.
5. Captura e câmera virtual.
6. Distribuição: app assinado e notarizado para Apple Silicon.

## Regras

- Não inclua SDKs, DLLs, modelos ou binários proprietários no Git.
- Isole código de plataforma e mantenha o diff pequeno para facilitar merges do upstream.
- Não afirme desempenho ou qualidade sem medição no hardware real.
- `AGENTS.md` traz as regras do Silivid. As regras originais do Windows estão em `docs/history/rules/AGENTS_before_silivid_2026-11-23.md`, e seus caminhos `E:/` não se aplicam aqui.
- Sem push, release ou PR para o upstream sem pedido explícito do usuário.
