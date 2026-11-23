# Fase 1: build nativo em macOS arm64

Estado em 2026-11-23, na branch `macos/port`. Ambiente: macOS 27.2.1 arm64, Apple clang 21, CMake 4.5.1, Ninja 1.13.2, Qt 6.12.0 e FFmpeg 9.0.2 do Homebrew.

## Como compilar e testar

```sh
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

Para Release, troque `debug` por `release`. A saída fica em `build-macos/debug` e `build-macos/release`.

## Resultado

- Os dois presets configuram e compilam do zero (74 passos cada, já com o atualizador).
- O CTest passa 24 de 24 entradas nos dois presets, em cinco execuções seguidas. São 20 executáveis de teste, o `veyra_macos_build_probe` e 3 passos que limpam diretórios temporários.
- O app Veyra **ainda não existe no macOS**. Não há janela, reprodução, GPU, áudio nem captura.
- A configuração Windows e o ramo Linux/OBS **não foram executados** nesta máquina. Os presets `x64-*` foram comparados com a base `c6633121` e estão idênticos. O `CMakeLists.txt` mudou em relação a essa base: inclui `cmake/SilividVersion.cmake` e um desvio `if(APPLE)` antes do ramo existente; gera erro de configuração se `VEYRA_UPDATE_PUBLIC_KEY` ou `VEYRA_UPDATE_TEST_PORT` forem definidas; em `veyra_update_checker`, troca `PortableUpdate.cpp` por `PortableUpdateDisabled.cpp`, adiciona `SemVer.cpp` e define `VEYRA_SILIVID_VERSION`; e `veyra_qml_bridge` recebe `SILIVID_VERSION` em `VEYRA_DISPLAY_VERSION`. Essas mudanças no ramo Windows não foram compiladas.

## O que compila

| Alvo | Conteúdo |
| --- | --- |
| `veyra_portable` | 15 fontes: resultado NGX (só tradução de códigos), leitores ONNX/TFLite, cadeia de efeitos, replay buffer, codec de paridade, tabelas de cor e SMAA, reset do pipeline, núcleo do Remote Play, migração de dados QML |
| `veyra_portable_media` | `PosterFrame` e `CaptureAudioDsp`, com FFmpeg |
| `veyra_portable_qt` | `PlaylistModel` e `QmlDiskCache`, com Qt Core |
| `veyra_update_checker` | Verificação de releases do Silivid e SemVer, com Qt Network. Só notifica e abre a página da release. O interruptor de canal e a ponte QML ainda não entram no build. Detalhes em `docs/macos/UPDATES.md` |
| `veyra_macos_build_probe` | Linka todos os objetos acima com `WHOLE_ARCHIVE`, confere as versões de FFmpeg e Qt e avalia uma expressão no motor QML. Não reproduz vídeo |

Testes executados: adaptive FG, audio gain, buffering gate, capture capability, color grade, effect availability, export timeline, frame pacing repair, game mode policy, HDR display state, parity, realtime preview, NR anti-flicker, NR auto controller, Remote Play core, Remote Play trigger filter, QML data, playlist, QML cache e update checker.

Único arquivo de teste alterado: `tests/unit/PlaylistModelTests.cpp`. Ele chama `flush()` antes de `setFileTime`, porque sem isso o flush no fechamento do arquivo apagava a data definida no POSIX e os testes de ordenação por data falhavam. A fixture agora também falha de forma explícita se não conseguir abrir seus arquivos JSON. Nenhuma asserção mudou.

## O que ficou desligado

Ligar qualquer opção abaixo no macOS gera erro de configuração.

| Opção | Parte |
| --- | --- |
| `VEYRA_ENABLE_D3D12`, `VEYRA_ENABLE_D3D12_DEBUG` | Direct3D 12, DXGI e shaders HLSL |
| `VEYRA_ENABLE_NGX`, `VEYRA_ENABLE_EXPERIMENTAL_DLSSNR`, `VEYRA_ENABLE_DLSS_SR`, `VEYRA_ENABLE_DLSS_FG` | NVIDIA NGX e DLSS |
| `VEYRA_ENABLE_NVOF` | NVIDIA Optical Flow |
| `VEYRA_ENABLE_DIRECTSHOW` | Câmera virtual DirectShow |
| `VEYRA_ENABLE_MEDIA_FOUNDATION` | Captura e decodificação Media Foundation |
| `VEYRA_ENABLE_WASAPI` | Áudio WASAPI |
| `VEYRA_ENABLE_WINDOWS_INSTALLER` | Instalador e pacote portátil |
| `VEYRA_ENABLE_REMOTEPLAY` | Backend Chiaki do PS5 |
| `VEYRA_ENABLE_MOONLIGHT` | Cliente Moonlight (transporte Winsock) |
| `VEYRA_ENABLE_XBOX` | Streaming Xbox |

Stubs: nenhum na Fase 1. O atualizador adicionou um, `src/ui/PortableUpdateDisabled.cpp`, que mantém o fluxo portátil do Windows desligado e recusa download e instalação.

Fontes que passam na análise de sintaxe, mas ficaram fora de propósito:

- `src/virtualcam/CameraPlaceholder.cpp` só retorna falha e não representa funcionalidade.
- `src/ngx/compat/hde/hde64.c` é um desmontador x86-64 usado pelos unlocks NVIDIA.
- `src/moonlight/{Client,Crypto,Pairing,Xml}.cpp` ficam fora porque o módulo Moonlight está desligado. O transporte dele (`Http.cpp`, `Discovery.cpp`) usa Winsock e `IdentityStore.cpp` usa Win32.

Testes que passam na sintaxe, mas não linkam:

- Usam `wmain`: subtitle (bitmap, format, loading, text), ANR settings, color look, color LUT, IEC 61937.
- Dependem de fontes Windows: AverMedia carrier, HDR output tuning, NVIDIA VRR profile, repair preset.

## Levantamento do código

Medição anterior ao atualizador. Análise de sintaxe dos 436 arquivos `.c`/`.cpp` de `src/`, `tests/`, `apps/` e `tools/` na árvore de trabalho (431 `.cpp` e 5 `.c`), incluindo o `tools/macos_build_probe/main.cpp`, ainda não rastreado. Ela conta só o **primeiro** erro de cada arquivo, então corrigir um bloqueio pode revelar outros. O erro pode vir de um include transitivo. A análise não usa os defines do CMake nem o moc, então parte dos "outros bloqueios" é limitação dela. Passar aqui não prova link nem execução.

| Situação | Arquivos |
| --- | ---: |
| Passam | 63 |
| Param em `windows.h` (qualquer caixa) | 238 |
| Param em `d3d12.h` | 104 |
| Outros bloqueios | 31 |

Comando, em bash, na raiz do repositório:

```bash
QT=""; for d in /opt/homebrew/opt/qt*/lib/*.framework; do QT="$QT -F$(dirname "$d") -idirafter $d/Headers"; done; export QT
check_one() {
  case "$1" in *.c) lang="-x c -std=c11" ;; *) lang="-x c++ -std=c++20" ;; esac
  out=$(clang $lang -arch arm64 -fsyntax-only -w -ferror-limit=1 -Iinclude -Isrc -I"$(dirname "$1")" $QT -idirafter /opt/homebrew/include "$1" 2>&1)
  if [ $? -eq 0 ]; then printf 'OK\t%s\t\n' "$1"; else printf 'FAIL\t%s\t%s\n' "$1" "$(printf '%s\n' "$out" | grep -m1 'error:')"; fi
}
export -f check_one
R="${TMPDIR:-/tmp}/silivid-survey.tsv"
git ls-files -co --exclude-standard -- 'src/*.c' 'src/*.cpp' 'tests/*.c' 'tests/*.cpp' 'apps/*.c' 'apps/*.cpp' 'tools/*.c' 'tools/*.cpp' \
  | xargs -P 8 -I{} bash -c 'check_one "$1"' _ {} > "$R"
F=$(grep -c '^FAIL' "$R"); W=$(grep '^FAIL' "$R" | grep -ciE "'windows\.h' file not found"); D=$(grep '^FAIL' "$R" | grep -c "'d3d12.h' file not found")
echo "total=$(wc -l < "$R") passam=$(grep -c '^OK' "$R") windows=$W d3d12=$D outros=$((F-W-D))"
```

## O que bloqueia a Fase 2

1. **Tipos de frame presos ao D3D12.** `FramePacket.h`, `FrameBatch.h` e `GpuPassUtils.h` incluem `d3d12.h` ou `windows.h`, e quase todo o pipeline depende deles. A abstração de GPU precisa começar aqui.
2. **`veyra_base` não compila.** `Log`, `FileIdentity`, `RuntimePaths` e `ThunkHook` usam Win32. `ColorLut` e `ColorLookStore` também, inclusive `bcrypt`. Log e caminhos de runtime precisam de implementação por plataforma.
3. **`std::wstring` em 206 arquivos de `src/`, `include/` e `apps/`, ou 339 somando `tests/` e `tools/`** (`git grep -l 'std::wstring' -- src include apps [tests tools] | wc -l`). `wchar_t` tem 32 bits no macOS e 16 no Windows, então caminhos e textos não podem ser convertidos por cast. É preciso definir o tipo de caminho antes de portar as APIs.
4. **Shaders.** São 81 arquivos HLSL em `shaders/` (68 `.hlsl` e 13 `.hlsli`) sem equivalente em Metal (`git ls-files 'shaders/*.hlsl' 'shaders/*.hlsli' | wc -l`). No repositório inteiro são 100 (86 `.hlsl` e 14 `.hlsli`), contando `tests/perf/` e `third_party/gpu-dis/` (`git ls-files '*.hlsl' '*.hlsli' | wc -l`).
5. **App QML não compilado.** A ponte `src/ui/bridge/*` e `QmlPlayerBridge` dependem do engine e de Win32, e `apps/veyra-qml/main.cpp` ainda não entra no build.
6. **Erro do clang em `SceneCadenceAnalyzer.h`.** O argumento padrão `const Config& config = {}` usa uma struct aninhada com inicializadores de membro, o que o MSVC aceita e o clang rejeita. Bloqueia a análise de cena e seu teste.
7. **Pontos de entrada `wmain`.** 119 arquivos de `tests/` usam `wmain` (`git grep -lw wmain -- tests | wc -l`) e precisam de um `main` portável. Desses, 8 já passam na análise de sintaxe e são os listados em "Testes que passam na sintaxe, mas não linkam".

## Decisão: pastas padrão sem migração

As pastas padrão de capturas e gravações passaram de `Veyra` para `Silivid`. Não há migração de arquivos porque o Silivid não tem instalações existentes. A pasta escolhida pelo usuário continua tendo prioridade.

## Pendências do binário Windows

Estas pendências bloqueiam um binário Windows, mas não uma pré-release só de código-fonte. O código Windows, o instalador e os empacotadores não foram compilados nem executados nesta máquina.

1. **Empacotadores.** `scripts/package-qml-release.py` só aceita versões `2.0.x` e `scripts/package-vendor-release.py` usa `2.0.3` como padrão, então a URL `silivid-v<versão>` da fonte correspondente ainda não representa uma versão real do Silivid. As URLs de dependências continuam no Veyra e precisam de uma fonte publicada para a versão empacotada.
2. **`scripts/installer/Veyra.iss`.** `DisplayName`, os campos `VersionInfo` e o nome do arquivo de saída ainda usam Veyra. `AppId` e `InstallId` foram mantidos e mudá-los é uma decisão, porque afeta upgrade e desinstalação. A montagem `{#AppVersion}.0` não foi verificada com uma versão alpha.
3. **`scripts/package-remoteplay-source.py:66`.** O texto do pacote ainda indica a fonte da aplicação na tag `v<versão>` de `Likely7/Veyra-NRVideo`.
4. **Perfil NVIDIA.** `src/gfx/NvidiaVrrProfile.cpp:120` ainda cria perfis chamados `Veyra VRR - <executável>`.
5. **Ícones em `qml/Veyra/sponsor/`.** Os 8 SVGs não têm mais uso em QML e foram mantidos porque `THIRD_PARTY_NOTICES.md` os credita. É preciso decidir sobre eles e sobre esses créditos antes de empacotar o app.
