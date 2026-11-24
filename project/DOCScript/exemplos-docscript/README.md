# Exemplos de DOC Script

Os arquivos desta pasta demonstram a sintaxe própria do DOC Script. Eles estão organizados do exemplo mais simples para os que usam entrada, decisões e laços.

Referências começam com maiúscula. Comentários usam parênteses, como `(Lê um valor.)`; precisam ser fechados e não podem ser aninhados. `/*` e `*/` marcam snippets e são removidos sem descartar o código entre eles. As palavras reservadas do português estão em `src/languages/pt/keywords.h`.

## Requisitos

Compile o DOC Script uma vez, a partir da raiz do projeto:

```bash
make
```

Os comandos abaixo também devem ser executados nessa pasta.

## Executar um exemplo

Use `run` para compilar e executar um programa:

```bash
./docscript run exemplos-docscript/ola_mundo.ds
```

Para fornecer a entrada de demonstração de um programa que usa `leia`, redirecione o arquivo `.in` correspondente:

```bash
./docscript run exemplos-docscript/media.ds < exemplos-docscript/media.in
```

Cada exemplo com entrada tem um arquivo `.out` com a saída esperada. Compare a saída executando:

```bash
./docscript run exemplos-docscript/media.ds < exemplos-docscript/media.in > /tmp/media.out
diff -u exemplos-docscript/media.out /tmp/media.out
```

Parênteses agrupadores ainda não são aceitos pelo parser. Por isso, `media.ds` calcula a média como `nota1 / 2.0 + nota2 / 2.0`.

`emit` gera explicitamente um arquivo C. Em compilações normais, `--save-c` preserva esse arquivo opcional; sem a opção, o C não é gravado em disco. `check` analisa sem executar:

```bash
./docscript emit exemplos-docscript/tabuada.ds -o /tmp/tabuada.c
./docscript run exemplos-docscript/tabuada.ds --save-c
./docscript check exemplos-docscript/tabuada.ds
```

## Exemplos incluídos

| Arquivo | O que demonstra |
|---|---|
| `ola_mundo.ds` | estrutura mínima e saída |
| `media.ds` | leitura de dados, operações e tipo `real` |
| `par_ou_impar.ds` | resto da divisão e decisão `se`/`senao` |
| `tabuada.ds` | repetição com `enquanto` |
| `contagem.ds` | repetição com `para` e leitura do limite |

## Usar no editor

Na raiz do projeto, inicie o servidor:

```bash
./docscript serve
```

Abra <http://127.0.0.1:8080/> no navegador e copie o conteúdo de um arquivo `.ds` para o editor. Para programas com `leia`, preencha o painel de entrada padrão com os mesmos valores mostrados no arquivo `.in`.
