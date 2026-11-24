#include "strbuf.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void sb_init(StrBuf *sb)
{
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

void sb_free(StrBuf *sb)
{
    free(sb->data);
    sb_init(sb);
}

void sb_reserve(StrBuf *sb, size_t extra)
{
    size_t need = sb->len + extra + 1;
    if (need <= sb->cap) {
        return;
    }
    size_t cap = sb->cap ? sb->cap : 64;
    while (cap < need) {
        cap *= 2;
    }
    sb->data = xrealloc(sb->data, cap);
    sb->cap = cap;
}

void sb_putc(StrBuf *sb, char c)
{
    sb_reserve(sb, 1);
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
}

void sb_write(StrBuf *sb, const char *s, size_t n)
{
    sb_reserve(sb, n);
    memcpy(sb->data + sb->len, s, n);
    sb->len += n;
    sb->data[sb->len] = '\0';
}

void sb_puts(StrBuf *sb, const char *s)
{
    sb_write(sb, s, strlen(s));
}

void sb_printf(StrBuf *sb, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    va_list copy;
    va_copy(copy, ap);
    int n = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (n < 0) {
        va_end(ap);
        return;
    }
    sb_reserve(sb, (size_t)n);
    vsnprintf(sb->data + sb->len, (size_t)n + 1, fmt, ap);
    va_end(ap);
    sb->len += (size_t)n;
}

void sb_indent(StrBuf *sb, int level)
{
    for (int i = 0; i < level; i++) {
        sb_puts(sb, "    ");
    }
}

char *sb_take(StrBuf *sb)
{
    if (!sb->data) {
        char *empty = xstrdup("");
        return empty;
    }
    char *data = sb->data;
    sb_init(sb);
    return data;
}

const char *sb_cstr(StrBuf *sb)
{
    if (!sb->data) {
        return "";
    }
    return sb->data;
}
