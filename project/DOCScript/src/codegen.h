#ifndef DOCSCRIPT_CODEGEN_H
#define DOCSCRIPT_CODEGEN_H

#include "ast.h"

/* Gera o código C do programa. O buffer retornado deve ser liberado com free. */
char *codegen_program(const Program *program);

/* Converte um identificador DOC Script em um identificador válido em C. */
char *codegen_ident(const char *name);

#endif
