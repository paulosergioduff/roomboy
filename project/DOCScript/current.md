Quero reorganizar esse projeto.

Pretendo escrever a linguagem do zero, com pastas mais organizadas na raiz,
renomear a pasta portugal2 para DOCScript, remover esses arquivos sh, manter o milestone. Ao invés da pasta linguagem natural, deixar todos os arquivos na raiz do projeto. REmover o preview.html, pois agora teremos o editor.html

Nova organização da sintaxe:

* Toda palavra reservada será organizada em um parser com indicação da língua a ser usada para o contribuidor da linguagem docscript. Por exemplo, nesse caso pt. Assim será possível fazer versões do docscript em outras línguas
* Toda Referencia deve iniciar com letra Maiuscula. Nome de variavel, nome de funçao, etc.
* Parenteses continuam sendo comentários
* Os símbolos /* e */ não constituem comentários, mas marcadores de snippets. E não fazem nada no código.
* Para funções receberem valor usa-se a palavra reservada Recebe + variaveis, separadas por virgulas, e ponto encerra funcao recebe .
Ex:
Função calcular recebe valor e percentual.
    Retorna valor * percentual.
* Faça o arquivo C de compilação ser feito em memória, não no disco. Mantenha as opções no CLI se quiser gerar uma saída em C
* Reescreva o parser com as novas regras, deixe src organizada, os arquivos estão ali espalhados, incompreensíveis de entender. Deixe organizados e funcionando. Reescreva exemplos e o curso com a nova sintaxe, e citando todas essas regras no curso. Teste tudo, e compile tudo (mas organizado as compilações). Enfim, commite tudo e dÊ push

## TODO — experimento PORT com Snake (2026-11-25)

Resumo da conversa:

* O objetivo do PORT é substituir por completo pelo menos um arquivo de código do projeto. Não manter o original C e uma cópia DOCScript lado a lado: isso duplicaria as fontes (por exemplo, 200 arquivos virariam 400).
* A integração final deve compilar fontes DOCScript como parte do build do jogo, deixando o código C consumidor trabalhar com os objetos resultantes sem conhecer DOCScript.
* Bibliotecas C existentes devem continuar sendo usadas. DOCScript deve poder importar headers com `Arquivos:` e deixar o compilador C resolver as declarações e bibliotecas, sem reimplementá-las.
* Lua foi citado como analogia de linguagem de extensão; o experimento atual é tradução DOCScript para C no build, não embedding/runtime Lua-like.

Experimento local realizado com Snake:

* `game.c` chama `SnakeStepX` e `SnakeStepY`, implementadas em `src/snake_logic.ds`; o CMake chama `docscript --module`, compila o C gerado e liga esse objeto ao executável.
* O módulo importa `game.h` com `Arquivos:`. As bibliotecas Win32 originais permanecem no link do jogo.
* O parser ganhou corpos de função com `Inicio ... FimFuncao` e `Retorna`; o modo `--module` exporta funções sem gerar `main`.
* Build MinGW do jogo, execução das funções de direção e suíte do DOCScript passaram. Esse arranjo com um `.ds` auxiliar é somente um experimento de integração, não o formato final desejado para PORT.

Próximos passos:

1. Definir o fluxo PORT para que um arquivo DOCScript substitua seu correspondente C, sem duplicar fontes.
2. Compilar `.ds` diretamente para objeto durante o build (sem exigir um `.c` gerado persistente) e integrar a regra ao sistema de build.
3. Formalizar imports de headers/bibliotecas C via `Arquivos:` e validar chamadas externas contra os headers importados.
4. Converter um arquivo completo do Snake para DOCScript e compilar o jogo com essa substituição.


