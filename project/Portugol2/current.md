# Estado atual

Objetivo: manter o DOC Script documentado e testado, com publicação por etapas.

Situação: o compilador gera o executável sem gravar C intermediário no projeto por padrão. `--save-c`, `-o`/`--output` e o comando explícito `emit` preservam/geram o arquivo C. Os exemplos próprios do DOC Script estão em `linguagem-natural/exemplos-docscript/`. Comentários entre parênteses são retirados pelo tokenizador antes do parser. Os C gerados mantidos na pasta de exemplos e o antigo `exemplo.c` foram removidos; snapshots C da suíte continuam como fixtures de teste. Build, compilação padrão, preservação opcional, `-c`, geração explícita, cinco exemplos e suíte foram validados.

Última ação: o commit funcional `566e24b` e o histórico com títulos revisados foram enviados a `origin/main` por force-with-lease. `make test` passou (36/0), os cinco exemplos passaram e a API do editor compilou/executou um programa. As únicas referências nominais à inspiração ficaram na homenagem do README e no arquivo de história.

Próxima ação: use `--save-c` ou `-o` quando quiser manter o C gerado; a compilação normal deixa apenas o executável.
