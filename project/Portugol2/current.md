# Estado atual

Objetivo: manter o DOC Script documentado e testado, com publicação por etapas.

Situação: o compilador gera o executável sem gravar C intermediário no projeto por padrão. `--save-c`, `-o`/`--output` e `emit` preservam/geram o arquivo C explicitamente. Os exemplos próprios do DOC Script estão em `linguagem-natural/exemplos-docscript/`. O curso interativo de dez aulas está em `linguagem-natural/curso/`, acessível em `/curso/` e pelo link no editor. Navegação, progresso local, editor e execução pela API foram validados. O servidor está ativo em `http://127.0.0.1:8080/`. Comentários entre parênteses são retirados pelo tokenizador antes do parser.

Última ação: testei os assets do editor e do curso no servidor em `127.0.0.1:8080`; compilei e executei as dez aulas pela API (todas com código 0), validei `node --check curso/app.js` e rodei `make test` (36 aprovados, 0 falhas). O servidor permanece ativo para testar o curso.

Próxima ação: revisar o diff e publicar a pasta do curso, a rota do servidor e a documentação.
