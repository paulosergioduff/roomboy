# Estado atual

Objetivo: conferir a integração da sintaxe Portugol, concluir o que estiver pendente, verificar o funcionamento e publicar as etapas concluídas.

Situação: a implementação já contém dois dialetos. DOC Script usa palavras em português e sintaxe própria; o dialeto Portugol usa `programa`, `funcao inicio`, chaves, `leia`/`escreva`/`escreval`, condicionais e laços. Comentários nos dois dialetos usam parênteses e são removidos pelo tokenizador antes do parser. A pasta `linguagem-natural/exemplos-portugol/` reúne cinco exemplos comentados, com instruções, entradas e saídas esperadas. Todos os exemplos passaram por compilação, execução e comparação com seus `.out`; `make test` também passou (34 aprovados, 0 falhas). O servidor do editor está ativo em `http://127.0.0.1:8080/`.

Última ação: commit `1e68b49` (`Documenta comentarios entre parenteses`) enviado para `origin/main`. Comentários entre parênteses são descartados pelo tokenizador antes do parser nos dois dialetos; os cinco exemplos Portugol e um caso golden agora demonstram essa regra. C e saídas mantiveram-se iguais, e `make test` passou (34 aprovados, 0 falhas).

Próxima ação: usar os exemplos documentados em `linguagem-natural/exemplos-portugol/`.
