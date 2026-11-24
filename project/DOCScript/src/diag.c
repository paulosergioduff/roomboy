#include "diag.h"

#include <stdarg.h>
#include <string.h>

static void copy_field(char *dst, size_t size, const char *src)
{
    if (!src) {
        dst[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= size) {
        len = size - 1;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

void diag_clear(Diagnostic *d)
{
    memset(d, 0, sizeof(*d));
    d->category = DIAG_SINTATICO;
}

void diag_set(Diagnostic *d, DiagCategory cat, int line, int col,
              const char *message)
{
    diag_clear(d);
    d->category = cat;
    d->has_location = 1;
    d->line = line;
    d->col = col;
    copy_field(d->message, sizeof(d->message), message);
}

void diag_set_full(Diagnostic *d, DiagCategory cat, int line, int col,
                   const char *found, const char *expected,
                   const char *message, const char *hint)
{
    diag_set(d, cat, line, col, message);
    copy_field(d->found, sizeof(d->found), found);
    copy_field(d->expected, sizeof(d->expected), expected);
    copy_field(d->hint, sizeof(d->hint), hint);
}

void diag_set_file(Diagnostic *d, const char *file)
{
    copy_field(d->file, sizeof(d->file), file);
}

const char *diag_category_name(DiagCategory cat)
{
    switch (cat) {
    case DIAG_LEXICO:
        return "LÉXICO";
    case DIAG_SINTATICO:
        return "SINTÁTICO";
    case DIAG_SEMANTICO:
        return "SEMÂNTICO";
    case DIAG_ARQUIVO:
        return "ARQUIVO";
    case DIAG_BACKEND:
        return "BACKEND";
    case DIAG_EXECUCAO:
        return "EXECUÇÃO";
    }
    return "DESCONHECIDO";
}

void diag_render(const Diagnostic *d, StrBuf *out)
{
    sb_printf(out, "ERRO DE COMPILAÇÃO: %s\n", d->message);
    sb_printf(out, "  categoria: %s\n", diag_category_name(d->category));
    if (d->has_location) {
        if (d->file[0]) {
            sb_printf(out, "  local: %s:%d:%d\n", d->file, d->line, d->col);
        } else {
            sb_printf(out, "  local: linha %d, coluna %d\n", d->line, d->col);
        }
    }
    if (d->found[0]) {
        sb_printf(out, "  encontrado: %s\n", d->found);
    }
    if (d->expected[0]) {
        sb_printf(out, "  esperado: %s\n", d->expected);
    }
    if (d->hint[0]) {
        sb_printf(out, "  dica: %s\n", d->hint);
    }
}

void diag_print(const Diagnostic *d, FILE *stream)
{
    StrBuf sb;
    sb_init(&sb);
    diag_render(d, &sb);
    fputs(sb_cstr(&sb), stream);
    sb_free(&sb);
}
