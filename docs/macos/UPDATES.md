# Atualizador do Silivid

O atualizador do Silivid só avisa que existe uma versão nova e abre a página da release. Ele não baixa nem instala nada e não usa mirrors.

**Estado em 2026-11-23:** o verificador compila e passa nos testes no macOS. O app ainda não existe nesta plataforma, então o interruptor e a ponte QML não foram compilados nem executados, e nenhuma release real foi lida.

## Versão e tags

- A versão vive só em `cmake/SilividVersion.cmake`. O valor atual é `0.1.0-alpha.1`.
- Uma release dessa versão **precisa** usar a tag `silivid-v0.1.0-alpha.1`.
- Tags sem o prefixo `silivid-v`, como as herdadas `v2.x`, são ignoradas mesmo que tenham uma release publicada. Uma tag `v0.1.0-alpha.1` também não conta.
- O projeto CMake continua `veyra 2.2.0`, mas a ponte QML, o atualizador e o `version.rc` do Windows usam a versão do Silivid.
- Os campos numéricos do Windows preservam a ordem SemVer no quarto campo: `alpha.N` = 10000+N, `beta.N` = 20000+N, `rc.N` = 30000+N e final = 40000. `0.1.0-alpha.1` vira `0,1,0,10001`, com a string completa em `FileVersion` e `ProductVersion` e a flag `VS_FF_PRERELEASE`.
- Por causa desse mapeamento, a versão do produto precisa ser `X.Y.Z` ou `X.Y.Z-(alpha|beta|rc).N`, com N até 9999 e cada campo até 65535. Outro formato gera erro de CMake.

## Como a verificação funciona

O verificador consulta somente `https://api.github.com/repos/davimf721/Silivid/releases?per_page=30`. O endpoint `/releases/latest` não serve aqui, porque nunca retorna pré-releases e responde 404 enquanto não houver release estável.

| Situação na resposta | Resultado |
| --- | --- |
| Tag sem `silivid-v` | Ignorada |
| Rascunho | Ignorado nos dois canais |
| Pré-release no canal estável | Ignorada |
| Pré-release no canal pré-release | Candidata |
| Tag inválida em SemVer ou com build metadata | Rejeitada |
| Flag `prerelease` diferente do sufixo da tag | Rejeitada |
| `html_url` diferente de `https://github.com/davimf721/Silivid/releases/tag/<tag>` | Rejeitada |

Entre as candidatas vale a maior versão em SemVer, por exemplo `alpha.1 < alpha.2 < alpha.10 < beta.1 < 0.1.0`. O aviso só aparece se ela for maior que a versão instalada. Uma entrada rejeitada não bloqueia outra válida. Se nenhuma for válida e houver rejeitadas, o resultado é `invalid-release`.

Também falham de forma fechada: JSON inválido, resposta acima de 2 MiB, erro HTTP, timeout de 12 s e qualquer redirecionamento, que não é seguido.

O canal estável é o padrão. O canal pré-release é opt-in: o interruptor em Sobre (`set-update-prerelease`) grava a preferência `updatePrereleaseChannel`, e a ponte QML a aplica ao iniciar. Esse caminho está no código, mas **ainda não foi compilado nem executado**.

Quando há versão nova, o botão de Sobre chama `openRelease()`, que abre a página da release no navegador.

## Fluxo portátil desligado

`src/ui/PortableUpdateDisabled.cpp` substitui `PortableUpdate.cpp` no build. Esse stub mantém a interface usada pelo QML, sempre informa que não há pacote e recusa `downloadAndSchedule()`. O código de download não é linkado.

Os mirrors GH-Proxy e GHFast foram removidos, e `assetUrl()` retorna vazio. No Windows, configurar `VEYRA_UPDATE_PUBLIC_KEY` ou `VEYRA_UPDATE_TEST_PORT` agora gera erro de CMake. Nenhuma chave foi gerada.

## Testes

```sh
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

`veyra_update_checker_tests` usa só um servidor em `127.0.0.1` e não acessa a internet.

Diagnóstico manual contra a API real, fora do CTest:

```sh
build-macos/debug/veyra_update_checker_tests --live
build-macos/debug/veyra_update_checker_tests --live --prerelease
build-macos/debug/veyra_update_checker_tests --live --prerelease --current 0.1.0-alpha.0
```

O código de saída é 0 para `current` ou `available`, e 1 para falha.

## Limites

- Em 2026-11-23 o repositório não tinha releases. O `--live` retornou `current` nos dois canais, o que valida TLS e o acesso à API, mas **não** valida a leitura de uma release real.
- Só as 30 releases mais recentes são avaliadas. A API sem autenticação tem limite de requisições, e um 403 aparece como falha da verificação.
- O verificador compila e roda no macOS, mas o app completo ainda não existe nesta plataforma. O interruptor de canal, a ponte QML e o build Windows **não foram compilados nem executados**.
- `SettingsPage.qml` mantém a linha de download do fluxo portátil. Ela fica oculta porque o stub nunca informa um pacote disponível, e `downloadAndSchedule()` é recusado.
- `scripts/acceptance/smoke-portable-update.py` sai com código 1 e a mensagem `DISABLED`, porque o fluxo que ele testava está desligado.
- O `version.rc` gerado foi conferido como texto no macOS, mas **não** foi compilado por `rc.exe` nem inspecionado em um `.exe`.
- `scripts/acceptance/merge-claude-fixes-build.py` e `sr-visual-build.py` preenchem o template com os nomes antigos e agora param no próprio `assert`. São builds históricos fixados em 2.0.6.
