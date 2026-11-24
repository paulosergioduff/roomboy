#ifndef DOCSCRIPT_STRBUF_H
#define DOCSCRIPT_STRBUF_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

void sb_init(StrBuf *sb);
void sb_free(StrBuf *sb);
void sb_reserve(StrBuf *sb, size_t extra);
void sb_putc(StrBuf *sb, char c);
void sb_puts(StrBuf *sb, const char *s);
void sb_write(StrBuf *sb, const char *s, size_t n);
void sb_printf(StrBuf *sb, const char *fmt, ...);
void sb_indent(StrBuf *sb, int level);

/* Devolve o buffer NUL-terminado (malloc) e reinicia o StrBuf. */
char *sb_take(StrBuf *sb);

/* Ponteiro para o conteúdo NUL-terminado (válido enquanto não houver escrita). */
const char *sb_cstr(StrBuf *sb);

#endif
