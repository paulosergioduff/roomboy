#ifndef DOCSCRIPT_TOKEN_H
#define DOCSCRIPT_TOKEN_H

#include <stdbool.h>

#include "diag.h"

typedef enum {
    TK_EOF,
    TK_STRING,
    TK_CHAR,
    TK_NUMBER,
    TK_ASSIGN,
    TK_OP,
    TK_DOT,
    TK_COLON,
    TK_COMMA,
    TK_SEMI,
    TK_LBRACE,
    TK_RBRACE,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_IDENT,
    TK_KW
} TokenKind;

typedef struct {
    TokenKind kind;
    char *value;
    int line;
    int col;
} Token;

typedef struct {
    Token *items;
    int count;
    int capacity;
} TokenList;

/* Tokeniza o fonte. Em caso de erro, preenche diag e retorna false. */
bool tokenize(const char *source, TokenList *out, Diagnostic *diag);
bool tokenize_language(const char *source, const char *language,
                       TokenList *out, Diagnostic *diag);

/* Compara o token com uma palavra-chave (ASCII, sem diferenciar caixa). */
bool token_is_kw(const Token *token, const char *keyword);

/* Texto legível do token para diagnósticos. */
const char *token_describe(const Token *token);

void token_list_free(TokenList *list);

#endif
