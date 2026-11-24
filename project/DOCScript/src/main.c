#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "codegen.h"
#include "diag.h"
#include "parser.h"
#include "process.h"
#include "semantic.h"
#include "server.h"
#include "token.h"
#include "util.h"

#define DOCSCRIPT_VERSION "0.2.0"

enum {
    EXIT_OK = 0,
    EXIT_COMPILE = 1,
    EXIT_USAGE = 2,
    EXIT_BACKEND = 3,
    EXIT_IO = 4,
    EXIT_EXECUCAO = 5
};

typedef struct {
    const char *verb;
    const char *input;
    const char *c_out;
    const char *exe_out;
    const char *cc;
    const char *language;
    int only_c;
    int save_c;
    int print_c;
    int run;
    int check;
    int quiet;
    int warnings;
} Options;

static void usage(FILE *stream)
{
    fputs("DOC Script — compilador da linguagem natural procedural\n"
          "\n"
          "Uso:\n"
          "  docscript <arquivo.ds> [opções]        compila e gera o executável\n"
          "  docscript emit <arquivo.ds> [opções]   gera apenas o código C\n"
          "  docscript check <arquivo.ds>           apenas analisa o programa\n"
          "  docscript run <arquivo.ds> [opções]    compila e executa\n"
          "  docscript serve [opções]               inicia o servidor do editor\n"
          "\n"
          "Opções:\n"
          "      --save-c                preserva o C gerado ao compilar\n"
          "      --print-c               também imprime o C gerado em stdout\n"
          "  -o, --output <arquivo.c>   caminho do C gerado (também o preserva)\n"
          "      --exe <arquivo>        caminho do executável (padrão: nome sem "
          "extensão)\n"
          "      --cc <compilador>      compilador C (padrão: $CC ou cc)\n"
          "      --language <idioma>    conjunto de palavras reservadas (padrão: pt)\n"
          "  -c, --only-c               não chama o compilador C\n"
          "      --run                  executa depois de compilar\n"
          "      --check                apenas analisa (não gera C)\n"
          "  -q, --quiet                não imprime mensagens de sucesso\n"
          "      --no-warnings          não imprime avisos semânticos\n"
          "  -h, --help                 mostra esta ajuda\n"
          "  -v, --version              mostra a versão\n"
          "\n"
          "Servidor do editor:\n"
          "  docscript serve --port 8080 --host 127.0.0.1\n",
          stream);
}

static char *change_extension(const char *path, const char *extension)
{
    const char *slash = strrchr(path, '/');
    const char *dot = strrchr(path, '.');
    size_t base = strlen(path);
    if (dot && (!slash || dot > slash)) {
        base = (size_t)(dot - path);
    }
    size_t ext_len = strlen(extension);
    char *out = xmalloc(base + ext_len + 1);
    memcpy(out, path, base);
    memcpy(out + base, extension, ext_len + 1);
    return out;
}

static char *path_for_exec(const char *path)
{
    if (strchr(path, '/')) {
        return xstrdup(path);
    }
    size_t len = strlen(path);
    char *out = xmalloc(len + 3);
    out[0] = '.';
    out[1] = '/';
    memcpy(out + 2, path, len + 1);
    return out;
}

static int parse_options(int argc, char **argv, Options *options)
{
    memset(options, 0, sizeof(*options));
    options->warnings = 1;
    options->language = "pt";

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
            usage(stdout);
            exit(EXIT_OK);
        }
        if (!strcmp(arg, "-v") || !strcmp(arg, "--version")) {
            printf("DOC Script %s\n", DOCSCRIPT_VERSION);
            exit(EXIT_OK);
        }
        if (!strcmp(arg, "check") || !strcmp(arg, "emit") ||
            !strcmp(arg, "build") || !strcmp(arg, "run")) {
            options->verb = arg;
            continue;
        }
        if (!strcmp(arg, "-o") || !strcmp(arg, "--output")) {
            if (++i >= argc) {
                fprintf(stderr, "docscript: %s exige um caminho.\n", arg);
                return 0;
            }
            options->c_out = argv[i];
            options->save_c = 1;
            continue;
        }
        if (!strcmp(arg, "--save-c")) {
            options->save_c = 1;
            continue;
        }
        if (!strcmp(arg, "--print-c")) {
            options->print_c = 1;
            continue;
        }
        if (!strcmp(arg, "--exe")) {
            if (++i >= argc) {
                fprintf(stderr, "docscript: --exe exige um caminho.\n");
                return 0;
            }
            options->exe_out = argv[i];
            continue;
        }
        if (!strcmp(arg, "--cc")) {
            if (++i >= argc) {
                fprintf(stderr, "docscript: --cc exige um nome.\n");
                return 0;
            }
            options->cc = argv[i];
            continue;
        }
        if (!strcmp(arg, "--language")) {
            if (++i >= argc) {
                fprintf(stderr, "docscript: --language exige um código, como pt.\n");
                return 0;
            }
            options->language = argv[i];
            continue;
        }
        if (!strcmp(arg, "-c") || !strcmp(arg, "--only-c")) {
            options->only_c = 1;
            options->save_c = 1;
            continue;
        }
        if (!strcmp(arg, "--run")) {
            options->run = 1;
            continue;
        }
        if (!strcmp(arg, "--check")) {
            options->check = 1;
            continue;
        }
        if (!strcmp(arg, "-q") || !strcmp(arg, "--quiet")) {
            options->quiet = 1;
            continue;
        }
        if (!strcmp(arg, "--no-warnings")) {
            options->warnings = 0;
            continue;
        }
        if (arg[0] == '-') {
            fprintf(stderr, "docscript: opção desconhecida '%s'.\n", arg);
            return 0;
        }
        if (options->input) {
            fprintf(stderr, "docscript: mais de um arquivo de entrada.\n");
            return 0;
        }
        options->input = arg;
    }

    if (options->verb) {
        if (!strcmp(options->verb, "check")) {
            options->check = 1;
        } else if (!strcmp(options->verb, "emit")) {
            options->only_c = 1;
            options->save_c = 1;
        } else if (!strcmp(options->verb, "run")) {
            options->run = 1;
        }
    }
    return 1;
}

static int report_io_error(const char *path, const char *action)
{
    Diagnostic diag;
    diag_clear(&diag);
    diag.category = DIAG_ARQUIVO;
    diag.has_location = 0;
    snprintf(diag.message, sizeof(diag.message), "Não foi possível %s '%s': %s.",
             action, path, strerror(errno));
    snprintf(diag.hint, sizeof(diag.hint),
             "Verifique se o caminho existe e se há permissão de leitura ou "
             "escrita.");
    diag_print(&diag, stderr);
    return EXIT_IO;
}

static int compile(const Options *options)
{
    size_t source_len = 0;
    char *source = read_file(options->input, &source_len);
    (void)source_len;
    if (!source) {
        return report_io_error(options->input, "ler");
    }

    Diagnostic diag;
    diag_clear(&diag);
    diag_set_file(&diag, options->input);

    TokenList tokens;
    if (!tokenize_language(source, options->language, &tokens, &diag)) {
        diag_set_file(&diag, options->input);
        diag_print(&diag, stderr);
        free(source);
        return EXIT_COMPILE;
    }
    free(source);

    Program program;
    if (!parse_program(&tokens, &program, &diag)) {
        diag_set_file(&diag, options->input);
        diag_print(&diag, stderr);
        token_list_free(&tokens);
        return EXIT_COMPILE;
    }
    token_list_free(&tokens);

    semantic_check(&program, options->input, options->warnings, stderr);

    if (options->check) {
        if (!options->quiet) {
            printf("Análise concluída: %d variável(is), %d instrução(ões).\n",
                   program.variable_count, program.body_count);
        }
        program_free(&program);
        return EXIT_OK;
    }

    char *code = codegen_program(&program);
    if (options->print_c) {
        fputs(code, stdout);
    }
    char *c_path = NULL;
    if (options->save_c) {
        c_path = options->c_out ? xstrdup(options->c_out)
                                : change_extension(options->input, ".c");
        if (write_file(c_path, code, strlen(code)) != 0) {
            int status = report_io_error(c_path, "gravar");
            free(code);
            free(c_path);
            program_free(&program);
            return status;
        }
        if (!options->quiet) {
            printf("Código C gerado: %s\n", c_path);
        }
    }

    if (options->only_c) {
        free(code);
        free(c_path);
        program_free(&program);
        return EXIT_OK;
    }

    char *exe_path = options->exe_out ? xstrdup(options->exe_out)
                                      : change_extension(options->input, "");
    const char *cc = options->cc ? options->cc : getenv("CC");
    if (!cc || !*cc) {
        cc = "cc";
    }

    char *cc_argv[] = {(char *)cc, "-std=c11", "-O2", "-Wall", "-Wextra",
                       "-x",       "c",        "-o",  exe_path, "-", NULL};
    ProcResult build;
    if (proc_run_capture(cc_argv, code, strlen(code), 0, &build) != 0) {
        fprintf(stderr, "docscript: falha ao iniciar o compilador C '%s'.\n",
                cc);
        free(c_path);
        free(code);
        free(exe_path);
        program_free(&program);
        return EXIT_BACKEND;
    }
    free(code);

    if (build.exit_code != 0) {
        diag_clear(&diag);
        diag.category = DIAG_BACKEND;
        diag.has_location = 0;
        snprintf(diag.message, sizeof(diag.message),
                 "O compilador C '%s' rejeitou o código gerado.", cc);
        if (options->save_c) {
            snprintf(diag.hint, sizeof(diag.hint),
                     "O código C foi preservado em '%s'. A saída do compilador "
                     "C está abaixo.", c_path);
        } else {
            snprintf(diag.hint, sizeof(diag.hint),
                     "Ative --save-c para preservar o código C gerado. A saída "
                     "do compilador C está abaixo.");
        }
        diag_print(&diag, stderr);
        if (build.err_len) {
            fputs(build.err, stderr);
        }
        if (build.out_len) {
            fputs(build.out, stderr);
        }
        proc_result_free(&build);
        free(c_path);
        free(exe_path);
        program_free(&program);
        return EXIT_BACKEND;
    }
    proc_result_free(&build);
    if (!options->quiet) {
        printf("Executável gerado: %s\n", exe_path);
    }

    int status = EXIT_OK;
    if (options->run) {
        char *run_path = path_for_exec(exe_path);
        char *run_argv[] = {run_path, NULL};
        int code_status = proc_run_inherit(run_argv);
        if (code_status < 0) {
            status = EXIT_EXECUCAO;
        } else {
            status = code_status;
        }
        free(run_path);
    }

    free(c_path);
    free(exe_path);
    program_free(&program);
    return status;
}

int main(int argc, char **argv)
{
    setlocale(LC_CTYPE, "");
    if (argc >= 2 && !strcmp(argv[1], "serve")) {
        return server_main(argc - 1, argv + 1);
    }

    Options options;
    if (!parse_options(argc, argv, &options)) {
        usage(stderr);
        return EXIT_USAGE;
    }
    if (!options.input) {
        usage(stderr);
        return EXIT_USAGE;
    }
    if (options.check && options.run) {
        fprintf(stderr, "docscript: --check não pode ser combinado com --run.\n");
        return EXIT_USAGE;
    }
    if (options.only_c && options.run) {
        fprintf(stderr,
                "docscript: --only-c não pode ser combinado com --run.\n");
        return EXIT_USAGE;
    }

    return compile(&options);
}
