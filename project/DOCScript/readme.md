# DOCScript

DOCScript é uma linguagem compilada com palavras reservadas em português. O código-fonte fica organizado diretamente na raiz do projeto.

## Sintaxe acordada

- O conjunto de palavras reservadas fica em `src/languages/<idioma>/`; `pt` é o idioma inicial. O tokenizador recebe a opção `--language pt`.
- Toda referência — variáveis, funções e demais nomes — começa com letra maiúscula.
- Comentários usam parênteses: `(comentário)`. Não podem ser aninhados.
- `/*` e `*/` são marcadores de snippets. O tokenizador remove apenas os marcadores; o conteúdo entre eles continua sendo código ativo.
- A declaração de parâmetros usa `Recebe`, separa os nomes por vírgula (ou `e`) e termina com ponto. O corpo retorna uma expressão com `Retorna`:

  ```text
  Função Calcular Recebe Valor e Percentual.
      Retorna Valor * Percentual.
  ```

  Uma chamada passa os valores com a mesma palavra: `Calcular Recebe 200 e 0.15.` (vírgula também é aceita).

- Comandos da forma tradicional terminam com ponto final. A sintaxe compacta atual também aceita blocos entre chaves e separadores opcionais.

O parser C suporta funções numéricas de retorno por expressão, parâmetros, chamadas, programas, variáveis, entrada e saída, expressões, decisões e laços. Referências iniciadas por minúsculas são rejeitadas pelo tokenizador.

## Organização

- `src/`: tokenizador, parser, AST, análise semântica, backend C, CLI e servidor local.
- `src/languages/pt/keywords.h`: palavras reservadas do português.
- `tests/`: casos, saídas esperadas e diagnósticos.
- `exemplos-docscript/`: programas de demonstração.
- `curso/`: curso interativo e editor integrado.
- `milestone/`: cópia histórica preservada.

## Compilar e usar

```sh
make
./docscript run exemplo.ds
./docscript check exemplo.ds --language pt
./docscript emit exemplo.ds -o /tmp/exemplo.c
./docscript serve
```

A compilação padrão envia o C gerado ao compilador C pela entrada padrão, sem criar um arquivo C intermediário. `--save-c` ou `-o/--output` preservam uma cópia quando desejado. `emit` é a opção explícita para gerar C. O editor local abre em <http://127.0.0.1:8080/> e o curso em <http://127.0.0.1:8080/curso/>.

## Testes

```sh
make clean all
make test
```

A suíte compara código C, saídas dos exemplos e diagnósticos. Os binários e objetos de build ficam ignorados pelo Git.
