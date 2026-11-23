# Exemplos em Portugol

Os arquivos desta pasta usam o dialeto Portugol aceito pelo DOC Script. Eles estão organizados do exemplo mais simples para os que usam entrada, decisões e laços.

Comentários nos dois dialetos são escritos entre parênteses. O tokenizador os remove antes do parser; por exemplo: `(Lê um valor.)`. Todo comentário precisa ser fechado e comentários aninhados não são aceitos.

## Requisitos

Compile o DOC Script uma vez, a partir da pasta `linguagem-natural`:

```bash
make
```

Os comandos abaixo também devem ser executados nessa pasta.

## Executar um exemplo

Use `run` para compilar e executar um programa:

```bash
./docscript run exemplos-portugol/ola_mundo.ln
```

Para fornecer a entrada de demonstração de um programa que usa `leia`, redirecione o arquivo `.in` correspondente:

```bash
./docscript run exemplos-portugol/media.ln < exemplos-portugol/media.in
```

Cada exemplo com entrada tem um arquivo `.out` com a saída esperada. Compare a saída executando:

```bash
./docscript run exemplos-portugol/media.ln < exemplos-portugol/media.in > /tmp/media.out
diff -u exemplos-portugol/media.out /tmp/media.out
```

Os parênteses para agrupar expressões ainda não são aceitos pelo parser. Por isso, `media.ln` calcula a média como `nota1 / 2.0 + nota2 / 2.0`.

`emit` gera somente o C, e `check` analisa o programa sem executá-lo:

```bash
./docscript emit exemplos-portugol/tabuada.ln -o /tmp/tabuada.c
./docscript check exemplos-portugol/tabuada.ln
```

## Exemplos incluídos

| Arquivo | O que demonstra |
|---|---|
| `ola_mundo.ln` | estrutura mínima e saída |
| `media.ln` | leitura de dados, operações e tipo `real` |
| `par_ou_impar.ln` | resto da divisão e decisão `se`/`senao` |
| `tabuada.ln` | repetição com `enquanto` |
| `contagem.ln` | repetição com `para` e leitura do limite |

## Usar no editor

Na pasta `linguagem-natural`, inicie o servidor:

```bash
./docscript serve
```

Abra <http://127.0.0.1:8080/> no navegador e copie o conteúdo de um arquivo `.ln` para o editor. Para programas com `leia`, preencha o painel de entrada padrão com os mesmos valores mostrados no arquivo `.in`.
