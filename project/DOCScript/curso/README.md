# Curso interativo de DOCScript

O curso tem dez aulas com navegação, exemplos editáveis, entrada padrão e execução no compilador local.

## Regras da linguagem

- O parser usa palavras reservadas do idioma selecionado; o padrão é `pt`, mantido em `src/languages/pt/keywords.h`.
- Toda referência começa com letra maiúscula: nomes de variáveis, funções e outros identificadores.
- Comentários de documentação usam `( ... )`; precisam ser fechados e não podem ser aninhados.
- `/*` e `*/` são marcadores de snippets. Eles próprios não fazem nada; o texto entre eles continua sendo código.
- Parâmetros de função são introduzidos por `Recebe`, separados por vírgula (ou `e`), e a lista termina com ponto. O corpo usa `Retorna`; a chamada passa valores após `Recebe`. Exemplo: `Calcular Recebe 200 e 0.15.`
- Instruções do dialeto tradicional terminam em ponto final. Blocos do dialeto compacto usam chaves.

O compilador implementa funções numéricas com corpo de uma expressão `Retorna`.

## Iniciar

Na raiz do projeto, compile e inicie o servidor:

```sh
make
./docscript serve
```

Abra <http://127.0.0.1:8080/curso/>. O servidor entrega a página e seus arquivos estáticos e usa `/api/compile` para executar o código no navegador.

## Usar

- Escolha uma aula na navegação lateral e avance na ordem sugerida.
- Edite o exemplo e pressione **Executar código** ou `Ctrl+Enter`.
- Para programas com `leia`, altere os valores na caixa **Entrada do programa**.
- Use **Restaurar** para recuperar o código inicial da aula.
- Marque aulas concluídas; o progresso e suas edições ficam salvos no navegador.

O curso é servido junto com o compilador local. Não abra `index.html` diretamente pelo sistema de arquivos, pois a execução depende da API local.
