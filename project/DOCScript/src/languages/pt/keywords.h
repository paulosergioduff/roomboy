#ifndef DOCSCRIPT_PT_KEYWORDS_H
#define DOCSCRIPT_PT_KEYWORDS_H

/* Palavras reservadas do parser em português. Para outro idioma, adicione
 * uma tabela equivalente e ligue-a à seleção de idioma do tokenizador. */
static const char *const DOCSCRIPT_PT_KEYWORDS[] = {
    "arquivos", "programa", "variaveis", "inicio", "fim", "inteiro",
    "real", "logico", "texto", "cadeia", "caracter", "leia",
    "escreva", "escreval", "se", "senao", "fimse", "entao",
    "verdadeiro", "falso", "e", "ou", "nao", "funcao", "enquanto",
    "para", "faca", "escolha", "caso", "defeito", "pare", "continue",
    "retorne", "recebe", "retorna", "fimfuncao"};
#define DOCSCRIPT_PT_KEYWORD_COUNT \
    (sizeof(DOCSCRIPT_PT_KEYWORDS) / sizeof(DOCSCRIPT_PT_KEYWORDS[0]))

#endif
