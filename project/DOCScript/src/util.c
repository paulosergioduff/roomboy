#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (!p) {
        fprintf(stderr, "ERRO FATAL: memória insuficiente.\n");
        exit(70);
    }
    return p;
}

void *xcalloc(size_t count, size_t size)
{
    void *p = calloc(count ? count : 1, size ? size : 1);
    if (!p) {
        fprintf(stderr, "ERRO FATAL: memória insuficiente.\n");
        exit(70);
    }
    return p;
}

void *xrealloc(void *ptr, size_t n)
{
    void *p = realloc(ptr, n ? n : 1);
    if (!p) {
        fprintf(stderr, "ERRO FATAL: memória insuficiente.\n");
        exit(70);
    }
    return p;
}

char *xstrdup(const char *s)
{
    if (!s) {
        return NULL;
    }
    size_t len = strlen(s);
    char *copy = xmalloc(len + 1);
    memcpy(copy, s, len + 1);
    return copy;
}

char *xstrndup(const char *s, size_t n)
{
    char *copy = xmalloc(n + 1);
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}

char *read_file(const char *path, size_t *len_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    char *data = xmalloc((size_t)size + 1);
    size_t read = fread(data, 1, (size_t)size, f);
    if (ferror(f)) {
        free(data);
        fclose(f);
        return NULL;
    }
    data[read] = '\0';
    fclose(f);
    if (len_out) {
        *len_out = read;
    }
    return data;
}

int write_file(const char *path, const char *data, size_t len)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        return -1;
    }
    size_t written = fwrite(data, 1, len, f);
    int failed = ferror(f);
    if (fclose(f) != 0) {
        failed = 1;
    }
    if (failed || written != len) {
        errno = EIO;
        return -1;
    }
    return 0;
}

int ascii_ieq(const char *a, const char *b)
{
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a++;
        unsigned char cb = (unsigned char)*b++;
        if (ca >= 'A' && ca <= 'Z') {
            ca = (unsigned char)(ca + 32);
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = (unsigned char)(cb + 32);
        }
        if (ca != cb) {
            return 0;
        }
    }
    return *a == '\0' && *b == '\0';
}

int ascii_ieq_n(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca >= 'A' && ca <= 'Z') {
            ca = (unsigned char)(ca + 32);
        }
        if (cb >= 'A' && cb <= 'Z') {
            cb = (unsigned char)(cb + 32);
        }
        if (ca != cb) {
            return 0;
        }
    }
    return 1;
}

size_t utf8_seq_len(unsigned char b)
{
    if (b < 0x80) {
        return 1;
    }
    if ((b & 0xE0) == 0xC0) {
        return 2;
    }
    if ((b & 0xF0) == 0xE0) {
        return 3;
    }
    if ((b & 0xF8) == 0xF0) {
        return 4;
    }
    return 1;
}

unsigned utf8_decode(const char **pp)
{
    const unsigned char *p = (const unsigned char *)*pp;
    unsigned cp;
    size_t n;

    if (p[0] < 0x80) {
        cp = p[0];
        n = 1;
    } else if ((p[0] & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80) {
        cp = ((unsigned)(p[0] & 0x1F) << 6) | (unsigned)(p[1] & 0x3F);
        n = 2;
    } else if ((p[0] & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 &&
               (p[2] & 0xC0) == 0x80) {
        cp = ((unsigned)(p[0] & 0x0F) << 12) |
             ((unsigned)(p[1] & 0x3F) << 6) | (unsigned)(p[2] & 0x3F);
        n = 3;
    } else if ((p[0] & 0xF8) == 0xF0 && (p[1] & 0xC0) == 0x80 &&
               (p[2] & 0xC0) == 0x80 && (p[3] & 0xC0) == 0x80) {
        cp = ((unsigned)(p[0] & 0x07) << 18) |
             ((unsigned)(p[1] & 0x3F) << 12) |
             ((unsigned)(p[2] & 0x3F) << 6) | (unsigned)(p[3] & 0x3F);
        n = 4;
    } else {
        cp = p[0];
        n = 1;
    }

    *pp += n;
    return cp;
}

static unsigned fold_cp(unsigned cp)
{
    if (cp >= 'A' && cp <= 'Z') {
        return cp + 32;
    }
    /* Latin-1 Supplement: À-Þ (exceto ×) dobram para minúsculas. */
    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) {
        return cp + 32;
    }
    return cp;
}

int ident_ieq(const char *a, const char *b)
{
    const char *pa = a;
    const char *pb = b;

    while (*pa && *pb) {
        unsigned ca = fold_cp(utf8_decode(&pa));
        unsigned cb = fold_cp(utf8_decode(&pb));
        if (ca != cb) {
            return 0;
        }
    }
    return *pa == '\0' && *pb == '\0';
}
