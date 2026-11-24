#ifndef DOCSCRIPT_DIAG_H
#define DOCSCRIPT_DIAG_H

#include <stdio.h>

#include "strbuf.h"

typedef enum {
    DIAG_LEXICO,
    DIAG_SINTATICO,
    DIAG_SEMANTICO,
    DIAG_ARQUIVO,
    DIAG_BACKEND,
    DIAG_EXECUCAO
} DiagCategory;

typedef struct {
    DiagCategory category;
    int has_location;
    int line;
    int col;
    char file[256];
    char found[160];
    char expected[192];
    char message[512];
    char hint[512];
} Diagnostic;

void diag_clear(Diagnostic *d);
void diag_set(Diagnostic *d, DiagCategory cat, int line, int col,
              const char *message);
void diag_set_full(Diagnostic *d, DiagCategory cat, int line, int col,
                   const char *found, const char *expected,
                   const char *message, const char *hint);
void diag_set_file(Diagnostic *d, const char *file);
const char *diag_category_name(DiagCategory cat);
void diag_render(const Diagnostic *d, StrBuf *out);
void diag_print(const Diagnostic *d, FILE *stream);

#endif
