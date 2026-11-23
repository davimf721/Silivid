# Regras do Silivid para agentes

Atualizado em 2026-11-23. Silivid é um fork GPL-3.0 do [Veyra](https://github.com/Likely7/Veyra-NRVideo) para macOS em Apple Silicon. As regras originais do Veyra para Windows estão preservadas em [`docs/history/rules/AGENTS_before_silivid_2026-11-23.md`](docs/history/rules/AGENTS_before_silivid_2026-11-23.md). Caminhos `E:/`, processos e autorizações descritos ali não se aplicam a este projeto.

## Estado real

- No macOS funcionam apenas o build portável, a sonda de build e os 24 itens do CTest.
- Não há app, janela, reprodução, GPU, áudio ou captura no macOS.
- O atualizador só pode notificar e abrir a página da release em `davimf721/Silivid`, com tags `silivid-v<versão>`. Não adicione download, instalação automática ou mirrors. O interruptor de canal e a ponte QML ainda não foram compilados nem executados, e nenhuma release real foi lida. Não o descreva como funcionando no app. Detalhes em `docs/macos/UPDATES.md`.
- Antes de trabalhar, leia [`docs/macos/PHASE1.md`](docs/macos/PHASE1.md) e [`docs/macos/PORTING.md`](docs/macos/PORTING.md). O índice está em [`docs/README.md`](docs/README.md).

## Git e publicação

- A branch de trabalho é `macos/port`. A `main` fica limpa para acompanhar o upstream.
- Confira status, branch e worktrees antes de começar. Não sobrescreva trabalho não commitado de outra pessoa ou agente.
- Não faça push, release, PR para o upstream nem mudança nas configurações do GitHub sem pedido explícito do usuário.
- Não envie mensagens nem comente em issues ou PRs de terceiros sem pedido explícito.

## Build e testes

```sh
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

- Para Release, troque `debug` por `release`. A saída fica em `build-macos/`, ignorado pelo Git.
- Mantenha repositório e builds em APFS, em `~/Developer`. Não use o SSD externo exFAT.
- Testes usam diretórios privados. Não altere configuração real do usuário, drivers, credenciais ou ambiente global.
- As opções exclusivas do Windows devem continuar gerando erro de configuração no macOS até existir uma implementação real.

## Disciplina de trabalho

1. Não minta nem agrade. Separe fato, inferência, resultado histórico, teste feito agora e relato do usuário.
2. Não afirme recurso, desempenho, qualidade ou compatibilidade sem comprovação no Mac. Stub, quadro repetido ou FPS teórico não contam como prova.
3. Registre comandos, resultados, falhas e o que não foi verificado.
4. Faça a menor mudança que resolva o problema. Isole código de plataforma para facilitar merges do upstream.
5. Preserve o comportamento do Windows. Se ele não puder ser executado nesta máquina, diga que não foi verificado.
6. NVIDIA DLSS não existe no macOS. MetalFX não deve ser descrito como equivalente.

## Documentação

- Documentos com o aviso `silivid-inherited-notice` descrevem o Veyra para Windows. Não os trate como estado do Silivid.
- Registre o trabalho do macOS em `docs/macos/`.
- Mantenha README, roadmap e este arquivo coerentes com o que foi comprovado.

## Licenças, créditos e binários

- Nunca remova ou altere créditos, avisos de copyright, licenças, atribuições ao Veyra ou links de código-fonte correspondente exigidos pela GPL. Preserve `LICENSE`, `licenses/`, `THIRD_PARTY_NOTICES.md` e `docs/history/`.
- SDKs, DLLs, modelos, headers, LIBs, capturas, credenciais e configurações proprietárias não entram no Git, no LFS, em recursos nem em base64.
- Não modifique, reassine, disfarce ou extraia binários de jogos ou caches de driver.
- O usuário pode substituir DLLs por conta própria. O software não deve recusá-las por hash ou assinatura fixa, e componentes distribuídos por padrão precisam de auditoria de origem, manifesto e licença, arquivo por arquivo. Não amplie o escopo de distribuição pública sem autorização. Os recursos ANR Beta continuam fora de pacotes públicos.
- Preserve o FFmpeg com patches, seu código-fonte correspondente, patches e licenças.
- Ao adaptar código aberto, prefira implementações maduras e de licença compatível. Registre repositório de origem, commit fixo, licença e modificações por arquivo.
- ReShade e add-ons RenoDX não entram na linha principal nem em pacotes. Componentes combinados com Chiaki seguem a AGPLv3 e sua exceção OpenSSL.
- A música de fundo autorizada não entra no Git nem em ZIPs de código-fonte. Preserve seu aviso e manifesto próprios.
