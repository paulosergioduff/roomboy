#ifndef DOCSCRIPT_SEMANTIC_H
#define DOCSCRIPT_SEMANTIC_H

#include <stdio.h>

#include "ast.h"

/* Verificações semânticas não bloqueantes. Emite avisos em out quando
 * warnings != 0. */
void semantic_check(const Program *program, const char *file, int warnings,
                    FILE *out);

#endif
