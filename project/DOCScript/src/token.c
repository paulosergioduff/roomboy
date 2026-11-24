#include "token.h"

#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "languages/pt/keywords.h"

bool token_is_kw(const Token *token, const char *keyword)
{
    if (token->kind != TK_KW) {
        return false;
    }
    if (!strcmp(keyword, "funcao") &&
        (!strcmp(token->value, "função") || !strcmp(token->value, "Função"))) {
        return true;
    }
    return ascii_ieq(token->value, keyword);
}

static bool is_keyword(const char *value, size_t len, const char *language)
{
    if (strcmp(language, "pt") != 0) {
        return false;
    }
    if (len == strlen("função") &&
        (!memcmp(value, "função", len) || !memcmp(value, "Função", len))) {
        return true;
    }
    for (size_t i = 0; i < DOCSCRIPT_PT_KEYWORD_COUNT; i++) {
        const char *kw = DOCSCRIPT_PT_KEYWORDS[i];
        if (strlen(kw) == len && ascii_ieq_n(kw, value, len)) {
            return true;
        }
    }
    return false;
}

static bool is_ident_start(unsigned char c)
{
    return isalpha(c) || c == '_' || c >= 0x80;
}

static bool is_ident_continue(unsigned char c)
{
    return isalnum(c) || c == '_' || c >= 0x80;
}

static bool identifier_starts_uppercase(const char *value, size_t len)
{
    if (!len) {
        return false;
    }
    unsigned char first = (unsigned char)value[0];
    if (first < 0x80) {
        return isupper(first) != 0;
    }
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t wc;
    size_t n = mbrtowc(&wc, value, len, &state);
    if (n == (size_t)-1 || n == (size_t)-2 || n == 0) {
        return false;
    }
    return towupper(wc) == (wint_t)wc && towlower(wc) != (wint_t)wc;
}

static void advance_position(const char *text, size_t len, int *line, int *col)
{
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\n') {
            (*line)++;
            *col = 1;
        } else if ((c & 0xC0) != 0x80) {
            (*col)++;
        }
    }
}

static void push_token(TokenList *list, TokenKind kind, const char *value,
                       size_t len, int line, int col)
{
    if (list->count == list->capacity) {
        list->capacity = list->capacity ? list->capacity * 2 : 32;
        list->items = xrealloc(list->items, (size_t)list->capacity * sizeof(Token));
    }
    Token *token = &list->items[list->count++];
    token->kind = kind;
    token->value = xstrndup(value, len);
    token->line = line;
    token->col = col;
}

bool tokenize_language(const char *source, const char *language,
                       TokenList *out, Diagnostic *diag)
{
    if (!language || strcmp(language, "pt") != 0) {
        diag_set_full(diag, DIAG_LEXICO, 1, 1, language ? language : "(vazio)",
                      "idioma disponível: pt", "Idioma do parser não disponível.",
                      "Adicione um conjunto de palavras reservadas em src/languages/<idioma>.");
        return false;
    }
    out->items = NULL;
    out->count = 0;
    out->capacity = 0;

    size_t len = strlen(source);
    size_t pos = 0;
    int line = 1;
    int col = 1;

    while (pos < len) {
        unsigned char c = (unsigned char)source[pos];
        int start_line = line;
        int start_col = col;

        if (isspace(c)) {
            size_t start = pos;
            while (pos < len && isspace((unsigned char)source[pos])) {
                pos++;
            }
            advance_position(source + start, pos - start, &line, &col);
            continue;
        }

        if (c == '(') {
            size_t start = pos;
            pos++;
            while (pos < len && source[pos] != ')') {
                pos++;
            }
            if (pos >= len) {
                char found[8];
                snprintf(found, sizeof(found), "'('");
                diag_set_full(diag, DIAG_LEXICO, start_line, start_col, found,
                              "')' para fechar o comentário",
                              "Comentário iniciado com '(' não foi fechado.",
                              "Comentários em DOC Script ficam entre parênteses; "
                              "feche-o com ')'.");
                return false;
            }
            pos++; /* consome ')' */
            advance_position(source + start, pos - start, &line, &col);
            continue;
        }

        /* Delimitadores de snippet: o conteúdo permanece no fluxo léxico;
         * apenas os marcadores não produzem tokens. */
        if ((c == '/' && pos + 1 < len && source[pos + 1] == '*') ||
            (c == '*' && pos + 1 < len && source[pos + 1] == '/')) {
            pos += 2;
            advance_position(source + pos - 2, 2, &line, &col);
            continue;
        }

        if (c == '"') {
            size_t start = pos;
            pos++;
            bool closed = false;
            while (pos < len) {
                if (source[pos] == '\\' && pos + 1 < len) {
                    pos += 2;
                    continue;
                }
                if (source[pos] == '"') {
                    pos++;
                    closed = true;
                    break;
                }
                pos++;
            }
            if (!closed) {
                diag_set_full(diag, DIAG_LEXICO, start_line, start_col, "\"",
                              "aspas de fechamento",
                              "Texto iniciado com aspas não foi fechado.",
                              "Feche o texto com uma segunda aspa dupla (\").");
                return false;
            }
            push_token(out, TK_STRING, source + start, pos - start, start_line,
                       start_col);
            advance_position(source + start, pos - start, &line, &col);
            continue;
        }

        if (c == '\'') {
            size_t start = pos;
            pos++;
            bool closed = false;
            while (pos < len) {
                if (source[pos] == '\\' && pos + 1 < len) {
                    pos += 2;
                    continue;
                }
                if (source[pos] == '\'') {
                    pos++;
                    closed = true;
                    break;
                }
                pos++;
            }
            if (!closed) {
                diag_set_full(diag, DIAG_LEXICO, start_line, start_col, "'",
                              "aspas simples de fechamento",
                              "Caractere iniciado com aspas simples não foi "
                              "fechado.",
                              "Feche o caractere com uma segunda aspa simples "
                              "(').");
                return false;
            }
            push_token(out, TK_CHAR, source + start, pos - start, start_line,
                       start_col);
            advance_position(source + start, pos - start, &line, &col);
            continue;
        }

        if (isdigit(c)) {
            size_t start = pos;
            while (pos < len && isdigit((unsigned char)source[pos])) {
                pos++;
            }
            if (pos + 1 < len && source[pos] == '.' &&
                isdigit((unsigned char)source[pos + 1])) {
                pos++;
                while (pos < len && isdigit((unsigned char)source[pos])) {
                    pos++;
                }
            }
            push_token(out, TK_NUMBER, source + start, pos - start, start_line,
                       start_col);
            advance_position(source + start, pos - start, &line, &col);
            continue;
        }

        if (c == '<' && pos + 1 < len && source[pos + 1] == '-') {
            push_token(out, TK_ASSIGN, source + pos, 2, start_line, start_col);
            pos += 2;
            advance_position(source + pos - 2, 2, &line, &col);
            continue;
        }

        if ((c == '+' || c == '-' || c == '*' || c == '/') && pos + 1 < len &&
            source[pos + 1] == '=') {
            push_token(out, TK_ASSIGN, source + pos, 2, start_line, start_col);
            pos += 2;
            advance_position(source + pos - 2, 2, &line, &col);
            continue;
        }

        if ((c == '+' || c == '-') && pos + 1 < len &&
            source[pos + 1] == c) {
            push_token(out, TK_OP, source + pos, 2, start_line, start_col);
            pos += 2;
            advance_position(source + pos - 2, 2, &line, &col);
            continue;
        }

        if ((c == '>' || c == '<' || c == '=' || c == '!') && pos + 1 < len &&
            source[pos + 1] == '=') {
            push_token(out, TK_OP, source + pos, 2, start_line, start_col);
            pos += 2;
            advance_position(source + pos - 2, 2, &line, &col);
            continue;
        }

        if (c == '=' || c == '<' || c == '>' || c == '+' || c == '*' ||
            c == '/' || c == '-' || c == '%') {
            push_token(out, TK_OP, source + pos, 1, start_line, start_col);
            pos++;
            advance_position(source + pos - 1, 1, &line, &col);
            continue;
        }

        if (c == '{' || c == '}' || c == '[' || c == ']' || c == ';') {
            TokenKind kind = TK_LBRACE;
            if (c == '}') {
                kind = TK_RBRACE;
            } else if (c == '[') {
                kind = TK_LBRACKET;
            } else if (c == ']') {
                kind = TK_RBRACKET;
            } else if (c == ';') {
                kind = TK_SEMI;
            }
            push_token(out, kind, source + pos, 1, start_line, start_col);
            pos++;
            advance_position(source + pos - 1, 1, &line, &col);
            continue;
        }

        if (c == '.') {
            push_token(out, TK_DOT, source + pos, 1, start_line, start_col);
            pos++;
            advance_position(source + pos - 1, 1, &line, &col);
            continue;
        }

        if (c == ':') {
            push_token(out, TK_COLON, source + pos, 1, start_line, start_col);
            pos++;
            advance_position(source + pos - 1, 1, &line, &col);
            continue;
        }

        if (c == ',') {
            push_token(out, TK_COMMA, source + pos, 1, start_line, start_col);
            pos++;
            advance_position(source + pos - 1, 1, &line, &col);
            continue;
        }

        if (is_ident_start(c)) {
            size_t start = pos;
            pos++;
            while (pos < len && is_ident_continue((unsigned char)source[pos])) {
                pos++;
            }
            size_t ident_len = pos - start;
            TokenKind kind = is_keyword(source + start, ident_len, language) ? TK_KW
                                                                   : TK_IDENT;
            if (kind == TK_IDENT &&
                !identifier_starts_uppercase(source + start, ident_len)) {
                diag_set_full(diag, DIAG_LEXICO, start_line, start_col,
                              "identificador iniciado com minúscula",
                              "referência iniciada com letra maiúscula",
                              "Referências devem começar com letra maiúscula.",
                              "Inicie nomes de variáveis, funções e demais referências com maiúscula.");
                return false;
            }
            push_token(out, kind, source + start, ident_len, start_line,
                       start_col);
            advance_position(source + start, ident_len, &line, &col);
            continue;
        }

        char found[16];
        if (isprint(c)) {
            snprintf(found, sizeof(found), "'%c'", c);
        } else {
            snprintf(found, sizeof(found), "byte 0x%02X", c);
        }
        diag_set_full(diag, DIAG_LEXICO, start_line, start_col, found,
                      "início de instrução, número, texto ou identificador",
                      "Caractere inesperado no código-fonte.",
                      "Remova o caractere ou substitua-o por um símbolo válido "
                      "da linguagem.");
        return false;
    }

    push_token(out, TK_EOF, "", 0, line, col);
    return true;
}

bool tokenize(const char *source, TokenList *out, Diagnostic *diag)
{
    return tokenize_language(source, "pt", out, diag);
}

const char *token_describe(const Token *token)
{
    return token->value ? token->value : "";
}

void token_list_free(TokenList *list)
{
    for (int i = 0; i < list->count; i++) {
        free(list->items[i].value);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}
