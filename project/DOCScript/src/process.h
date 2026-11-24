#ifndef DOCSCRIPT_PROCESS_H
#define DOCSCRIPT_PROCESS_H

#include <stddef.h>

typedef struct {
    int exit_code; /* -1 quando terminou por sinal */
    int signal;
    int timed_out;
    char *out;
    char *err;
    size_t out_len;
    size_t err_len;
} ProcResult;

/* Executa argv capturando stdout/stderr. stdin_data pode ser NULL.
 * timeout_ms <= 0 significa sem limite. Retorna 0 se o processo foi executado. */
int proc_run_capture(char *const argv[], const char *stdin_data,
                     size_t stdin_len, int timeout_ms, ProcResult *result);

/* Executa argv herdando stdin/stdout/stderr. Retorna o código de saída. */
int proc_run_inherit(char *const argv[]);

void proc_result_free(ProcResult *result);

#endif
