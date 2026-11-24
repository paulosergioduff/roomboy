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


