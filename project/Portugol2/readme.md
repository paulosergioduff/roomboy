# DOC Script

DOC Script é uma linguagem de programação de sintaxe em português, inspirada no Portugol, concebida para aproximar a expressão humana da programação compilada. A proposta combina instruções formais com comentários em linguagem natural, mantendo regras sintáticas e semânticas verificáveis por um compilador.

O compilador atual é escrito em C e traduz um subconjunto procedural para C, chamando o compilador C automaticamente. O editor Ace incluído no projeto conversa com um servidor HTTP local para compilar e executar os programas. Este documento registra as regras estabelecidas, o estado real da implementação e os próximos marcos.

## 1. Princípios da linguagem

- A sintaxe utiliza palavras-chave em português.
- A linguagem é compilada: o código DOC Script é analisado e traduzido para uma linguagem de destino.
- O primeiro backend é C.
- O código C gerado deve ser preservado como artefato de compilação, mesmo quando a geração do executável for automatizada.
- O compilador deve apresentar erros claros, com localização e explicações úteis para quem está aprendendo.
- A linguagem deve ser compreensível tanto por pessoas quanto por ferramentas de IA, sem depender de uma LLM para definir a semântica ou executar o programa.
- A arquitetura deve permitir backends e módulos futuros, sem exigir que todos os recursos das linguagens de destino sejam reimplementados no núcleo.

## 2. Regras sintáticas já definidas

### 2.1 Comentários

Comentários são delimitados por parênteses nos dois dialetos do compilador. O tokenizador remove o conteúdo entre `(` e `)` antes de chamar o parser, então ele não é executado nem traduzido como instrução.

```text
(Recebe os dados do candidato.)
Leia idade.
```

Comentários não podem ser aninhados e precisam ser fechados; um comentário aberto gera erro léxico com a posição do `(`.

### 2.2 Fim de comando

O terminador de comando é o ponto final (`.`), e não o ponto e vírgula (`;`).

```text
Escreva "Olá, mundo".
idade <- 18.
```

O ponto também encerra declarações e cabeçalhos de estruturas.

### 2.3 Importação de arquivos

A palavra reservada `Arquivos:` declara arquivos ou módulos utilizados pelo programa.

```text
Arquivos: "Cadastro".
```

O parser reconhece, valida a forma e registra os nomes declarados. A resolução efetiva dos arquivos, a busca em diretórios de módulos e a ligação de suas declarações ainda não estão implementadas.

### 2.4 Estrutura básica de um programa

```text
Programa "Exemplo".

Variaveis
    nome: Texto.
    idade: Inteiro.
    renda: Real.
    autorizado: Logico.

Inicio
    (Corpo do programa.)
    Escreva "Olá".
Fim.
```

`Arquivos:`, `Programa` e a seção `Variaveis` são opcionais. O bloco `Inicio ... Fim.` é obrigatório.

### 2.5 Tipos atualmente reconhecidos

| Tipo DOC Script | Representação atual em C |
|---|---|
| `Inteiro` | `int` |
| `Real` | `double` |
| `Logico` | `int` (0 ou 1) |
| `Texto` | `char[256]` |

O suporte a `Texto` ainda é parcial: a leitura usa `fgets` com buffer fixo de 256 bytes e mantém o caractere de nova linha. A leitura e a manipulação de texto precisam de evolução e testes antes de serem consideradas completas.

### 2.6 Instruções atualmente reconhecidas

```text
Leia idade.
Escreva "Mensagem".
resultado <- idade + 1.

Se idade >= 18 entao.
    Escreva "Maior de idade".
Senao.
    Escreva "Menor de idade".
FimSe.
```

O subconjunto atual inclui:

- declaração de variáveis;
- leitura com `Leia`;
- saída com `Escreva`;
- atribuição com `<-`;
- condição `Se`, `Senao` e `FimSe`, inclusive aninhada;
- valores lógicos `verdadeiro` e `falso`;
- operadores aritméticos, relacionais e lógicos.

### 2.7 Expressões e precedência

A precedência implementada, do menor para o maior vínculo, é:

| Nível | Operadores |
|---|---|
| 1 | `ou` |
| 2 | `e` |
| 3 | `=`, `==`, `!=`, `>`, `<`, `>=`, `<=` |
| 4 | `+`, `-` |
| 5 | `*`, `/` |
| 6 | unários `-`, `+`, `nao` |

Operadores binários associam à esquerda. `=` é o operador de igualdade; a atribuição usa `<-`.

```text
resultado <- idade >= 18 e renda > 1000.
```

### 2.8 Limitações sintáticas atuais

- Não há parênteses de agrupamento: `(a + b) / 2` ainda não é aceito.
- Não há laços (`Enquanto`, `Para`) nem funções/procedimentos.
- Não há vetores, matrizes ou registros.
- Não há validação completa de tipos: uma atribuição entre tipos incompatíveis pode gerar C inválido e ser reportada apenas pelo compilador C.

A aceitação de um operador pelo parser não significa que todas as combinações de tipos já tenham validação semântica completa.

## 3. Paradigmas e unidades de compilação

A primeira etapa implementa somente o paradigma procedural.

A direção arquitetural definida é permitir diferentes módulos ou unidades de compilação por paradigma, sem misturar paradigmas no mesmo código-fonte inicialmente. Uma aplicação maior poderá, futuramente, integrar componentes compilados a partir de unidades DOC Script distintas, desde que compartilhem contratos e interfaces compatíveis.

Possíveis direções futuras, ainda não implementadas:

- procedural, inicialmente traduzido para C;
- orientação a objetos, com avaliação de destinos como C++ ou Rust;
- outros paradigmas por módulos próprios;
- módulos de capacidades que encapsulem bibliotecas externas;
- notação explícita para incorporar código nativo da linguagem de destino, com restrição ao backend correspondente.

Paradigma e linguagem de destino são conceitos separados. A escolha de um backend não deve, por si só, definir a semântica do paradigma.

## 4. Compilação e arquitetura

### 4.1 Fluxo implementado

```text
Código DOC Script (.ln)
        |
        v
Tokenizador (src/token.c)
        |
        v
Parser + tabela de variáveis (src/parser.c)
        |
        v
AST (src/ast.c)
        |
        v
Verificações semânticas não bloqueantes (src/semantic.c)
        |
        v
Backend C (src/codegen.c)
        |
        +----> Código C preservado (.c)
        |
        v
Compilador C (cc/gcc, configurável)
        |
        v
Executável nativo
```

O comando principal executa o fluxo completo, preserva o arquivo `.c` mesmo quando a compilação C falha e repassa a saída do compilador C ao usuário.

### 4.2 Organização do código

| Arquivo | Responsabilidade |
|---|---|
| `src/token.c` | Tokenizador com suporte a UTF-8 e comentários entre parênteses |
| `src/parser.c` | Parser recursivo descendente e tabela de variáveis |
| `src/ast.c` | Estruturas da AST e liberação de memória |
| `src/semantic.c` | Avisos semânticos não bloqueantes |
| `src/codegen.c` | Geração de C e transliteração de identificadores |
| `src/diag.c` | Formatação dos diagnósticos |
| `src/process.c` | Execução de subprocessos com captura de saída e tempo limite |
| `src/json.c` | Leitura e escrita do JSON usado pelo servidor |
| `src/server.c` | Servidor HTTP local do editor |
| `src/main.c` | Interface de linha de comando |
| `src/strbuf.c`, `src/util.c` | Infraestrutura de strings, arquivos e UTF-8 |

### 4.3 Uso rápido

```bash
make                              # compila o binário ./docscript
./docscript exemplo.ln            # gera exemplo.c e o executável exemplo
./docscript run exemplo.ln        # compila e executa
./docscript emit exemplo.ln -o exemplo.c   # gera apenas o C
./docscript check exemplo.ln      # apenas analisa (tokeniza e parseia)
make test                         # executa a suíte de testes
```

Opções principais:

```text
-o, --output <arquivo.c>   caminho do C gerado
    --exe <arquivo>        caminho do executável
    --cc <compilador>      compilador C (padrão: $CC ou cc)
-c, --only-c               não chama o compilador C
    --run                  executa depois de compilar
    --check                apenas analisa
-q, --quiet                não imprime mensagens de sucesso
    --no-warnings          não imprime avisos semânticos
```

Códigos de saída do compilador:

| Código | Significado |
|---|---|
| `0` | sucesso |
| `1` | erro léxico, sintático ou semântico |
| `2` | uso incorreto da linha de comando |
| `3` | falha do compilador C (backend) |
| `4` | falha de leitura/escrita de arquivo |
| `5` | falha ao iniciar o programa gerado |

Quando `--run` é usado, o código de saída é o do programa executado.

### 4.4 Servidor local do editor

```bash
./docscript serve                 # http://127.0.0.1:8080/
./docscript serve --port 9000
```

Rotas atendidas:

| Rota | Método | Função |
|---|---|---|
| `/` | GET | entrega `editor.html` |
| `/exemplo.ln` | GET | entrega o exemplo para o editor carregar |
| `/api/health` | GET | verificação de disponibilidade |
| `/api/compile` | POST | compila e, opcionalmente, executa |

Corpo de `/api/compile`:

```json
{
  "code": "Inicio\n    Escreva \"Olá\".\nFim.",
  "stdin": "20\n1500\n1\n",
  "mode": "run"
}
```

`mode` pode ser `run` (padrão) ou `emit` (apenas gera o C). A resposta contém `ok`, `stage`, `diagnostics`, `c`, `stdout`, `stderr`, `exitCode` e `timedOut`.

O servidor compila cada requisição em um diretório temporário e aplica tempo limite de 5 segundos à execução do programa do usuário. Ele é destinado ao uso local e não deve ser exposto publicamente.

### 4.5 Exemplos no dialeto Portugol

Os programas de demonstração do dialeto Portugol estão em [`linguagem-natural/exemplos-portugol/`](linguagem-natural/exemplos-portugol/), com instruções de uso, arquivos de entrada e saídas de referência. Compile primeiro com `make` na pasta `linguagem-natural`; em seguida, por exemplo:

```bash
cd linguagem-natural
./docscript run exemplos-portugol/ola_mundo.ln
./docscript run exemplos-portugol/media.ln < exemplos-portugol/media.in
```

Os exemplos também podem ser copiados para o editor local iniciado por `./docscript serve`. Consulte [`linguagem-natural/exemplos-portugol/README.md`](linguagem-natural/exemplos-portugol/README.md) para a lista completa e os comandos para cada arquivo.

## 5. Diagnósticos e experiência de aprendizagem

Os erros são apresentados com categoria, localização, token encontrado, expectativa, explicação e orientação:

```text
ERRO DE COMPILAÇÃO: Variável 'idade' não foi declarada.
  categoria: SEMÂNTICO
  local: exemplo.ln:2:10
  encontrado: 'idade'
  esperado: variável declarada na seção Variaveis
  dica: Declare a variável na seção Variaveis antes de usá-la.
```

Categorias usadas:

- `LÉXICO`: caracteres, textos e comentários malformados;
- `SINTÁTICO`: estrutura do programa e pontuação;
- `SEMÂNTICO`: variáveis não declaradas, duplicadas ou tipos inválidos;
- `ARQUIVO`: falhas de leitura/escrita;
- `BACKEND`: rejeição pelo compilador C;
- `EXECUÇÃO`: falhas ao iniciar o programa gerado.

Avisos não bloqueantes são emitidos em `stderr`:

- variável declarada e não utilizada;
- leitura de `Texto` (suporte parcial);
- comparação de textos com `=`/`!=` (compara endereços em C).

A ferramenta ajuda o aluno a compreender e corrigir o próprio código. Correção automática por LLM pode ser uma integração futura, mas não substitui o diagnóstico determinístico do compilador nem oculta os erros do estudante.

## 6. Editor Ace

O projeto inclui `editor.html`, uma interface inspirada no VS Code que utiliza Ace Editor.

O editor atual oferece:

- área editável com o exemplo `exemplo.ln`;
- numeração de linhas e destaque de sintaxe próprio para DOC Script;
- campo de entrada padrão (`stdin`) para os valores lidos por `Leia`;
- botões **Executar** (Ctrl+Enter) e **Gerar C**;
- painéis para saída do programa, código C gerado e diagnósticos;
- download do arquivo `.c` gerado;
- persistência local do código e da entrada no navegador;
- aviso explícito quando a página é aberta como arquivo local.

O HTML carrega Ace por CDN. Sem internet, o editor cai para um modo simples baseado em `textarea`, mantendo a compilação funcional quando servido por `./docscript serve`.

O destaque de sintaxe não substitui a validação do compilador.

## 7. Exemplo atual

O arquivo `exemplo.ln` contém um programa procedural de análise de elegibilidade. Ele lê idade, renda e existência de documentos, avalia as condições e imprime o resultado.

O arquivo `exemplo.c` é a tradução C gerada e é preservado para inspeção e comparação com o código DOC Script.

## 8. Estado atual do projeto

**Estágio: protótipo funcional com compilador em C, compilação automática e editor conectado.**

Já realizado:

- tokenizador, parser, AST, verificações semânticas e backend C implementados em C;
- tradução do exemplo procedural para C com paridade validada em relação ao protótipo Python anterior;
- chamada automática ao compilador C, com preservação do `.c` e repasse dos erros do backend;
- diagnósticos com categoria, linha, coluna, token encontrado, expectativa e dica;
- servidor HTTP local e editor Ace conectado à compilação e execução;
- suíte de testes com golden files de C, saídas esperadas e diagnósticos de erro;
- protótipo Python anterior arquivado em `legacy/compiler.py`, sem participação no fluxo normal.

Limitações conhecidas:

- não há parênteses de agrupamento;
- não há laços, funções ou procedimentos;
- importações são reconhecidas, mas não resolvidas/carregadas;
- o suporte a `Texto` é parcial (buffer fixo e nova linha preservada);
- a validação de tipos é incompleta;
- não há suporte implementado aos paradigmas OOP ou a outros backends;
- não há sistema de módulos, gerenciamento de dependências ou integração de componentes em linguagens diferentes;
- `current.sh` e `preview.html` pertencem ao protótipo anterior e não fazem parte do fluxo atual.

## 9. TODO / próximos marcos

1. ~~Compilar automaticamente para C e gerar o executável.~~ **Concluído.**
2. ~~Implementar o parser em C, em vez de Python.~~ **Concluído.**
3. ~~Remover `compiler.py` do fluxo principal.~~ **Concluído:** a referência foi arquivada em `legacy/compiler.py` e é usada apenas por `tests/generate-expected.sh`.
4. ~~Criar o diretório Git `milestone` e commitar o projeto.~~ **Concluído** com a identidade `paulosergioduff <paulosergioduff@gmail.com>`.
5. **Parênteses de agrupamento:** aceitar `(expressão)` no parser e refletir na AST/backend.
6. **Laços e funções:** definir e implementar `Enquanto`, `Para` e subprogramas, com testes.
7. **Resolução de `Arquivos:`:** localizar módulos, carregar declarações e reportar erros de importação.
8. **Completar `Texto`:** leitura sem nova linha, buffers configuráveis, concatenação e comparação por conteúdo.
9. **Validação de tipos:** impedir atribuições e comparações inválidas antes de gerar C.
10. **Ampliar a suíte de testes:** cobrir precedência, limites de diagnóstico e programas inválidos que hoje só falham no GCC.

## 10. Testes

```bash
make test
```

A suíte atual (`tests/run.sh`) verifica:

- o C gerado para cada programa em `tests/cases/*.ln` contra `tests/expected/*.c`;
- a saída de execução contra `tests/cases/*.out`, usando `*.in` como entrada;
- os diagnósticos de `tests/errors/*.ln` contra o texto esperado em `*.expect`.

Os golden files foram gerados a partir da implementação de referência em Python antes da migração. Para regenerá-los:

```bash
./tests/generate-expected.sh
```

Esse script exige `legacy/compiler.py` e `cc`; ele não faz parte do fluxo de compilação normal.

## 11. Convenções de desenvolvimento

- Preservar os arquivos de código-fonte DOC Script e o C gerado para permitir inspeção e depuração.
- Não declarar um recurso como suportado apenas porque o editor Ace o colore; a implementação deve existir no compilador e possuir testes.
- Evoluir a gramática incrementalmente, registrando cada regra neste documento.
- Separar tokenização, parsing, validação semântica, representação intermediária, backend e execução.
- Manter os diagnósticos do compilador independentes de qualquer integração com LLM.
- Ao migrar componentes de linguagem ou backend, criar testes de regressão para os exemplos existentes e para erros sintáticos conhecidos.

---

Este README descreve o estado e as decisões estabelecidas até o milestone atual. Recursos indicados como futuros ou pendentes não devem ser tratados como implementados.
