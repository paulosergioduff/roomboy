# Log de trabalho

## 2026-11-23

- Localizado `current.md`; não havia `log.md` nem `AGENTS.md` na pasta do projeto.
- Inspecionados o README, o Makefile, os fontes e os casos de teste. O projeto já inclui parser e backend de Portugol com testes para entrada/saída, caracteres e laços.
- Confirmado que há mudanças preexistentes no repositório fora de `Portugol2` (`Dockerfile` modificado e `find.sh` não rastreado); não fazem parte deste trabalho.
- `make test` inicial revelou quatro casos negativos de Portugol sem `.expect`; o runner avisava, mas não falhava.
- Criadas expectativas para atribuição inválida, declaração duplicada, função não suportada e variável não declarada.
- Atualizado `tests/run.sh` para considerar a ausência de expectativa como falha.
- Executado novamente `make test`: 34 passaram, 0 falharam, sem avisos.
- Commit `98eac89` (`Completa expectativas dos testes Portugol`) criado e enviado para `origin/main`.
- A publicação incluiu somente os sete arquivos da etapa; alterações preexistentes em `Dockerfile` e `find.sh` permaneceram fora.
- Próximo passo: verificar se há outras lacunas na implementação Portugol.
- Iniciado o servidor local do editor: `./docscript serve --host 127.0.0.1 --port 8080`. Endereço: http://127.0.0.1:8080/.
- Criados cinco exemplos em `linguagem-natural/exemplos-portugol/`: Olá mundo, média, par ou ímpar, tabuada e contagem.
- Documentados os comandos de build, análise, geração de C, execução com arquivos `.in`, comparação com `.out` e uso pelo editor.
- Observado e documentado que o parser ainda não permite parênteses para agrupar expressões; o exemplo da média foi escrito sem agrupamento.
- Validação: os cinco programas executaram e corresponderam aos respectivos `.out`; `make test` terminou com 34 aprovados e 0 falhas.
- Commit `6eaed6c` (`Adiciona exemplos da sintaxe Portugol`) enviado para `origin/main`.
- Os executáveis gerados durante o teste não foram incluídos no commit; a revisão automática bloqueou a remoção deles do workspace local.
- Confirmado no tokenizador: comentários entre parênteses são descartados antes do parser nos dois dialetos.
- Adicionados comentários em parênteses a cada exemplo Portugol e ao caso golden `portugol_basico.ln`; documentada a regra compartilhada.
- Revalidação: C gerado e saídas dos cinco exemplos permaneceram idênticos; `make test` passou com 34 aprovados e 0 falhas.
- Commit `1e68b49` (`Documenta comentarios entre parenteses`) enviado para `origin/main`.
