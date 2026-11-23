# Estado atual

Objetivo: manter o DOC Script documentado e testado, com publicação por etapas.

Situação: o compilador gera o executável sem gravar C intermediário no projeto por padrão. `--save-c`, `-o`/`--output` e `emit` preservam/geram o arquivo C explicitamente. Os exemplos próprios do DOC Script estão em `linguagem-natural/exemplos-docscript/`. O curso interativo de dez aulas está em `linguagem-natural/curso/`, acessível em `/curso/` e pelo link no editor. Navegação, progresso local, editor e execução pela API foram validados. O servidor está ativo em `http://127.0.0.1:8080/`. Comentários entre parênteses são retirados pelo tokenizador antes do parser.

Última ação: commit `4b92206` (`Adiciona curso interativo de DOC Script`) enviado para `origin/main`. O curso está ativo em `http://127.0.0.1:8080/curso/`; os assets foram servidos, as dez aulas executaram pela API, `node --check curso/app.js` passou e `make test` passou (36/0).

Próxima ação: abrir `http://127.0.0.1:8080/curso/` para testar as aulas no navegador.
