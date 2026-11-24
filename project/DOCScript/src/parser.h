#ifndef DOCSCRIPT_PARSER_H
#define DOCSCRIPT_PARSER_H

#include <stdbool.h>

#include "ast.h"
#include "diag.h"
#include "token.h"

/* Analisa a lista de tokens e monta a AST. Retorna false e preenche diag em erro. */
bool parse_program(const TokenList *tokens, Program *program, Diagnostic *diag);

#endif
