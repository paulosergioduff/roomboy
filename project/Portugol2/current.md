# Estado atual

Objetivo: manter o DOC Script documentado e testado, com publicação por etapas.

Situação: o compilador gera o executável sem gravar C intermediário no projeto por padrão. `--save-c`, `-o`/`--output` e o comando explícito `emit` preservam/geram o arquivo C. Os exemplos próprios do DOC Script estão em `linguagem-natural/exemplos-docscript/`. Comentários entre parênteses são retirados pelo tokenizador antes do parser. Os C gerados mantidos na pasta de exemplos e o antigo `exemplo.c` foram removidos; snapshots C da suíte continuam como fixtures de teste. Build, compilação padrão, preservação opcional, `-c`, geração explícita, cinco exemplos e suíte foram validados.

Última ação: implementei e validei a saída C opcional, renomeei os exemplos e testes para DOC Script e removi os C gerados das pastas ativas. `make test` passou (36/0), os cinco exemplos passaram, e a API do editor compilou/executou um programa. Reescrevi os títulos dos commits anteriores para retirar o nome da inspiração; apenas a documentação de homenagem e o arquivo de história o mantêm. O commit funcional atual é `566e24b`; ainda falta atualizar `origin/main` com o histórico reescrito.

Próxima ação: enviar o histórico reescrito para `origin/main` com force-with-lease, preservando as alterações locais preexistentes em `Dockerfile` e `find.sh`.
