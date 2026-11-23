# Log de trabalho

## 2026-11-23

- Localizado `current.md`; não havia `log.md` nem `AGENTS.md` na pasta do projeto.
- Inspecionados README, Makefile, fontes e testes do DOC Script.
- Confirmado que há mudanças preexistentes no repositório fora de `Portugol2` (`Dockerfile` modificado e `find.sh` não rastreado); não fazem parte deste trabalho.
- A suíte revelou casos negativos sem `.expect`; o runner foi ajustado para falhar se a expectativa não existir.
- Criadas expectativas para atribuição inválida, declaração duplicada, função não suportada e variável não declarada.
- Atualizado `tests/run.sh` para considerar a ausência de expectativa como falha.
- Executado novamente `make test`: 34 passaram, 0 falharam, sem avisos.
- Commit de correção dos testes negativos criado e enviado para `origin/main`.
- A publicação incluiu somente os sete arquivos da etapa; alterações preexistentes em `Dockerfile` e `find.sh` permaneceram fora.
- Próximo passo: avaliar outras lacunas da linguagem.
- Iniciado o servidor local do editor: `./docscript serve --host 127.0.0.1 --port 8080`. Endereço: http://127.0.0.1:8080/.
- Criados cinco exemplos em `linguagem-natural/exemplos-docscript/`: olá mundo, média, par ou ímpar, tabuada e contagem.
- Documentados os comandos de build, análise, geração de C, execução com arquivos `.in`, comparação com `.out` e uso pelo editor.
- Observado e documentado que o parser ainda não permite parênteses para agrupar expressões; o exemplo da média foi escrito sem agrupamento.
- Validação: os cinco programas executaram e corresponderam aos respectivos `.out`; `make test` terminou com 34 aprovados e 0 falhas.
- Commit de adição dos exemplos enviado para `origin/main`.
- Os executáveis gerados durante o teste não foram incluídos no commit; a revisão automática bloqueou a remoção deles do workspace local.
- Confirmado no tokenizador: comentários entre parênteses são descartados antes do parser nos dois dialetos.
- Adicionados comentários em parênteses a cada exemplo DOC Script e ao caso golden; documentada a regra compartilhada.
- Revalidação: C gerado e saídas dos cinco exemplos permaneceram idênticos; `make test` passou com 34 aprovados e 0 falhas.
- Commit `1e68b49` (`Documenta comentarios entre parenteses`) enviado para `origin/main`.
- Renomeados os exemplos e casos de teste para DOC Script; atualizados os identificadores dos programas de teste.
- CLI: compilação padrão usa arquivo C temporário removido ao fim; `--save-c` preserva o C ao lado do fonte; `-o`/`--output` preserva no caminho indicado; `emit` e `-c` continuam gerando C explicitamente.
- Removidos os `.c` gerados da pasta de exemplos e o antigo `linguagem-natural/exemplo.c`; os C em `tests/expected/` continuam como snapshots golden.
- `make test`: 36 passaram, 0 falharam. Os cinco exemplos foram executados e suas saídas comparadas; nenhuma compilação padrão deixou `.c` na pasta dos exemplos.
- Testado também `-c` com saída C e confirmadas as opções `--save-c` e `--output` na ajuda.
- Testada a API do editor em porta temporária: compilação e execução devolveram `ok`, com código de saída zero.
- Revisados os títulos dos commits anteriores para usar o nome DOC Script ou descrições neutras. A referência à inspiração ficou restrita à documentação de homenagem e ao registro histórico.
- Commit funcional após a reescrita: `566e24b`. Próxima ação: atualizar `origin/main` com force-with-lease.
