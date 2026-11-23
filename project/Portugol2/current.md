# Estado atual

Objetivo: manter o DOC Script documentado e testado, com publicação por etapas.

Situação: o compilador gera o executável sem gravar C intermediário no projeto por padrão. `--save-c`, `-o`/`--output` e o comando explícito `emit` preservam/geram o arquivo C. Os exemplos próprios do DOC Script estão em `linguagem-natural/exemplos-docscript/`. Comentários entre parênteses são retirados pelo tokenizador antes do parser. Os C gerados mantidos na pasta de exemplos e o antigo `exemplo.c` foram removidos; snapshots C da suíte continuam como fixtures de teste. Build, compilação padrão, preservação opcional, `-c`, geração explícita, cinco exemplos e suíte foram validados.

Última ação: validei `make test` (36 aprovados, 0 falhas), todos os cinco exemplos com suas saídas e os modos `--save-c`, `--output`, `emit` e `-c`. A compilação padrão não deixou arquivo `.c`; removi o exemplo gerado antigo e os `.c` gerados da pasta de exemplos. Ainda não commitei nem publiquei esta etapa.

Próxima ação: revisar diff, atualizar mensagens de commit publicadas para remover a referência nominal à inspiração e publicar depois da validação já realizada.
