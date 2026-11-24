#ifndef DOCSCRIPT_UTIL_H
#define DOCSCRIPT_UTIL_H

#include <stddef.h>

void *xmalloc(size_t n);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t n);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);

/* Lê um arquivo inteiro. Retorna NULL em caso de erro e preenche errno. */
char *read_file(const char *path, size_t *len_out);

/* Grava um arquivo inteiro. Retorna 0 em sucesso, -1 em erro. */
int write_file(const char *path, const char *data, size_t len);

/* Comparação ASCII sem diferenciar maiúsculas/minúsculas. */
int ascii_ieq(const char *a, const char *b);

/* Comparação ASCII sem diferenciar caixa, limitada a n bytes. */
int ascii_ieq_n(const char *a, const char *b, size_t n);

/* Comparação de identificadores com dobra de caixa (casefold) para UTF-8. */
int ident_ieq(const char *a, const char *b);

/* Decodifica um code point UTF-8 a partir de *pp e avança o ponteiro. */
unsigned utf8_decode(const char **pp);

/* Comprimento em bytes de uma sequência UTF-8 iniciada por b. */
size_t utf8_seq_len(unsigned char b);

#endif
