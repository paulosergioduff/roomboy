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
