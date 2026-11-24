# A primeira noite da DOC Script

Foi num domingo à noite, em 22 de novembro de 2026.

Eu tinha acabado de estudar usando um dos materiais do MIT OpenCourseWare. A questão educacional ainda estava na minha cabeça. Enquanto pensava no que tinha estudado, comecei a me perguntar como seria a formação da próxima geração de programadores.

Durante muito tempo, aprender a programar significou, entre outras coisas, sentar diante de um computador e escrever código. Digitar, errar, consultar a documentação, corrigir, testar e repetir. Esse contato direto com o código fazia parte do aprendizado. Mas o mercado estava caminhando cada vez mais para a automação dessas tarefas por meio de inteligência artificial.

Isso me trouxe uma preocupação: se as novas gerações passarem a delegar cada vez mais a escrita do código, de onde virá a experiência necessária para formar os próximos programadores seniores?

Havia outro problema que também me chamava a atenção. As IAs já conseguiam produzir grandes quantidades de código, mas revisar tudo aquilo podia se tornar uma tarefa difícil. Quanto maior o volume gerado, maior o esforço para compreender o que foi feito, conferir se corresponde à intenção original e identificar possíveis problemas.

Foi então que me lembrei de uma ideia antiga: criar uma espécie de português estruturado, tão próximo da linguagem natural quanto fosse possível, mas que ainda pudesse ser compilado. Um código que, por sua própria forma de escrita, também servisse como documentação. Algo que se aproximasse de um prompt, mas sem depender de uma interpretação probabilística para determinar o que o programa deveria executar.

Eu já conhecia os passos gerais para criar uma linguagem de programação. Sabia que seria necessário pensar em tokens, sintaxe, parser e geração de código. A ideia me animou imediatamente. Talvez não precisasse existir uma grande justificativa para começar. Valeria a pena simplesmente pela diversão de construir algo assim.

Então comecei.

Dessa vez, não seguiria o caminho antigo de passar a noite inteira implementando tudo manualmente. Eu tinha trabalho no dia seguinte. Em vez disso, especifiquei a tarefa para duas IAs: montar um pequeno tokenizador e parser, capazes de reconhecer uma linguagem inicial e traduzi-la para C.

Usei o Portugol como inspiração para a estrutura. Queria comandos reconhecíveis, palavras em português e uma organização que lembrasse o modo como se ensina lógica de programação. Acrescentei também uma regra que me parecia especialmente importante: comentários entre parênteses.

Essa escolha tinha um motivo. Um comentário entre parênteses poderia acompanhar uma instrução formal e explicar sua intenção em linguagem natural. Assim, o código poderia continuar sendo processado por um compilador, mas também oferecer contexto para quem estivesse lendo — humano ou modelo de linguagem. A própria escrita do programa ajudaria a explicar o programa.

A primeira pasta do projeto recebeu o nome `Portugol2`. O registro do diretório marca o início daquela experiência: **22/11/2026, às 20h38**.

Eu estava muito animado. A ideia já tinha deixado de ser apenas uma lembrança antiga e começava a ganhar forma em arquivos, regras e código executável. O primeiro protótipo era pequeno, mas havia algo especialmente divertido em ver uma linguagem escrita em português ser traduzida para C e depois compilada por uma ferramenta tradicional.

A empolgação não terminou quando fui dormir. Acordei de madrugada e voltei a escrever especificações. Novas possibilidades apareciam: módulos, diferentes paradigmas, integração com outras linguagens, aplicações nativas e até a possibilidade de desenvolver partes da própria linguagem usando DOC Script.

Naquela noite, ainda não havia uma grande arquitetura definida nem respostas para todas essas possibilidades. Havia uma experiência inicial, algumas regras e a vontade de descobrir até onde a ideia poderia chegar.

A DOC Script começou assim: de uma preocupação com o futuro do aprendizado de programação, de uma lembrança antiga sobre escrever código em português estruturado e da curiosidade de transformar essa ideia em algo que realmente pudesse ser compilado.

E, acima de tudo, começou porque parecia divertido demais para não tentar.
