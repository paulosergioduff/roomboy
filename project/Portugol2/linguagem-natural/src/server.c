#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include "server.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "json.h"
#include "process.h"
#include "strbuf.h"
#include "util.h"

#define MAX_HEADER_BYTES (64 * 1024)
#define MAX_BODY_BYTES (2 * 1024 * 1024)
#define MAX_INPUT_BYTES 4096
#define RUN_TIMEOUT_MS 5000
#define SESSION_TIMEOUT_MS 60000
#define SESSION_STALE_SECONDS (60 * 60)

#define SESSION_PREFIX "/tmp/docscript-sess-"
#define SESSION_ID_LEN 6

typedef struct {
    int port;
    const char *host;
    const char *editor;
    const char *example;
} ServerOptions;

typedef struct {
    char *dir;
    char *ln_path;
    char *c_path;
    char *exe_path;
    int ok;
    const char *stage;
    int exit_code;
    char *diagnostics;
    char *c_text;
} BuildResult;

static char g_self[4096];

static long long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

static int send_all(int fd, const char *data, size_t len)
{
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, data + sent, len - sent);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static void http_respond(int fd, int status, const char *content_type,
                         const char *body, size_t body_len)
{
    const char *reason = status == 200   ? "OK"
                         : status == 400 ? "Bad Request"
                         : status == 404 ? "Not Found"
                         : status == 405 ? "Method Not Allowed"
                         : status == 409 ? "Conflict"
                         : status == 413 ? "Payload Too Large"
                         : status == 500 ? "Internal Server Error"
                                         : "OK";

    char header[512];
    int header_len = snprintf(
        header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Cache-Control: no-store\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, reason, content_type, body_len);

    send_all(fd, header, (size_t)header_len);
    if (body && body_len) {
        send_all(fd, body, body_len);
    }
}

static void http_text(int fd, int status, const char *content_type,
                      const char *body)
{
    http_respond(fd, status, content_type, body, strlen(body));
}

static void http_error(int fd, int status, const char *message)
{
    StrBuf body;
    sb_init(&body);
    sb_puts(&body, "{\"ok\":false,\"message\":\"");
    char *escaped = json_escape_alloc(message);
    sb_puts(&body, escaped);
    free(escaped);
    sb_puts(&body, "\"}");
    http_respond(fd, status, "application/json; charset=utf-8",
                 sb_cstr(&body), body.len);
    sb_free(&body);
}

static int find_content_length(const char *headers)
{
    const char *p = headers;
    while (*p) {
        const char *line_end = strstr(p, "\r\n");
        size_t line_len = line_end ? (size_t)(line_end - p) : strlen(p);
        static const char name[] = "content-length:";
        size_t name_len = sizeof(name) - 1;
        if (line_len > name_len && ascii_ieq_n(p, name, name_len)) {
            const char *value = p + name_len;
            while (*value == ' ' || *value == '\t') {
                value++;
            }
            return atoi(value);
        }
        if (!line_end) {
            break;
        }
        p = line_end + 2;
    }
    return -1;
}

static int read_http_request(int fd, char **method_out, char **path_out,
                             char **body_out, size_t *body_len_out)
{
    StrBuf buf;
    sb_init(&buf);
    char chunk[8192];
    char *header_end = NULL;

    while (!header_end) {
        ssize_t n = read(fd, chunk, sizeof(chunk));
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            sb_free(&buf);
            return -1;
        }
        if (n == 0) {
            break;
        }
        sb_write(&buf, chunk, (size_t)n);
        header_end = strstr(sb_cstr(&buf), "\r\n\r\n");
        if (!header_end && buf.len > MAX_HEADER_BYTES) {
            sb_free(&buf);
            return -1;
        }
    }

    if (!header_end) {
        sb_free(&buf);
        return -1;
    }

    size_t header_len = (size_t)(header_end - sb_cstr(&buf)) + 4;
    char *headers = xstrndup(sb_cstr(&buf), header_len);

    char *method = NULL;
    char *path = NULL;
    const char *p = headers;
    const char *space1 = strchr(p, ' ');
    if (space1) {
        method = xstrndup(p, (size_t)(space1 - p));
        const char *space2 = strchr(space1 + 1, ' ');
        if (space2) {
            path = xstrndup(space1 + 1, (size_t)(space2 - space1 - 1));
        }
    }
    if (!method || !path) {
        free(method);
        free(path);
        free(headers);
        sb_free(&buf);
        return -1;
    }

    char *query = strchr(path, '?');
    if (query) {
        *query = '\0';
    }

    int content_length = find_content_length(headers);
    if (content_length < 0) {
        content_length = 0;
    }
    if (content_length > MAX_BODY_BYTES) {
        free(method);
        free(path);
        free(headers);
        sb_free(&buf);
        return -1;
    }

    StrBuf body;
    sb_init(&body);
    size_t available = buf.len - header_len;
    if (available > 0) {
        size_t take = available < (size_t)content_length ? available
                                                         : (size_t)content_length;
        sb_write(&body, buf.data + header_len, take);
    }
    while (body.len < (size_t)content_length) {
        ssize_t n = read(fd, chunk, sizeof(chunk));
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (n == 0) {
            break;
        }
        sb_write(&body, chunk, (size_t)n);
    }

    free(headers);
    sb_free(&buf);

    *method_out = method;
    *path_out = path;
    *body_out = sb_take(&body);
    *body_len_out = (size_t)content_length;
    return 0;
}

static void serve_file(int fd, const char *path, const char *content_type)
{
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        http_error(fd, 404, "Arquivo não encontrado.");
        return;
    }
    size_t len = 0;
    char *data = read_file(path, &len);
    if (!data) {
        http_error(fd, 500, "Falha ao ler o arquivo.");
        return;
    }
    http_respond(fd, 200, content_type, data, len);
    free(data);
}

static void resolve_self(const char *argv0, char *buffer, size_t size)
{
    if (argv0 && strchr(argv0, '/') && realpath(argv0, buffer) != NULL) {
        return;
    }
    ssize_t n = readlink("/proc/self/exe", buffer, size - 1);
    if (n > 0) {
        buffer[n] = '\0';
        return;
    }
    snprintf(buffer, size, "%s", argv0 && *argv0 ? argv0 : "docscript");
}

static char *path_join(const char *dir, const char *name)
{
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    char *out = xmalloc(dir_len + name_len + 2);
    memcpy(out, dir, dir_len);
    out[dir_len] = '/';
    memcpy(out + dir_len + 1, name, name_len + 1);
    return out;
}

/* ---------- Sessões interativas ---------- */

static int valid_session_id(const char *id)
{
    size_t len = strlen(id);
    if (len != SESSION_ID_LEN) {
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)id[i])) {
            return 0;
        }
    }
    return 1;
}

static int session_paths(const char *id, char *dir, size_t dir_size,
                         char *fifo, size_t fifo_size, char *exe,
                         size_t exe_size, char *stop, size_t stop_size,
                         char *lock, size_t lock_size)
{
    if (!valid_session_id(id)) {
        return -1;
    }
    int n = snprintf(dir, dir_size, "%s%s", SESSION_PREFIX, id);
    if (n < 0 || (size_t)n >= dir_size) {
        return -1;
    }
    snprintf(fifo, fifo_size, "%s/in.fifo", dir);
    snprintf(exe, exe_size, "%s/programa", dir);
    snprintf(stop, stop_size, "%s/stop", dir);
    snprintf(lock, lock_size, "%s/running", dir);
    return 0;
}

static void remove_session_files(const char *dir)
{
    if (!dir) {
        return;
    }
    static const char *const names[] = {"main.ln", "main.c", "programa",
                                        "in.fifo", "stop", "running"};
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        char *path = path_join(dir, names[i]);
        unlink(path);
        free(path);
    }
    rmdir(dir);
}

static void cleanup_stale_sessions(void)
{
    DIR *directory = opendir("/tmp");
    if (!directory) {
        return;
    }
    time_t cutoff = time(NULL) - SESSION_STALE_SECONDS;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strncmp(entry->d_name, "docscript-sess-", 15) != 0) {
            continue;
        }
        char *path = path_join("/tmp", entry->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode) &&
            st.st_mtime < cutoff) {
            remove_session_files(path);
        }
        free(path);
    }
    closedir(directory);
}

static void build_result_free(BuildResult *result)
{
    free(result->ln_path);
    free(result->c_path);
    free(result->exe_path);
    free(result->diagnostics);
    free(result->c_text);
    if (result->dir) {
        remove_session_files(result->dir);
    }
    free(result->dir);
    memset(result, 0, sizeof(*result));
}

static int build_source(const char *code, BuildResult *result)
{
    memset(result, 0, sizeof(*result));
    result->stage = "ok";

    char dir_template[] = SESSION_PREFIX "XXXXXX";
    char *dir = mkdtemp(dir_template);
    if (!dir) {
        result->stage = "io";
        return -1;
    }
    result->dir = xstrdup(dir);
    result->ln_path = path_join(dir, "main.ln");
    result->c_path = path_join(dir, "main.c");
    result->exe_path = path_join(dir, "programa");

    if (write_file(result->ln_path, code, strlen(code)) != 0) {
        result->stage = "io";
        return -1;
    }

    char *build_argv[] = {g_self,       "build", result->ln_path, "-o",
                          result->c_path, "--exe", result->exe_path,
                          "--quiet",   NULL};
    ProcResult build;
    if (proc_run_capture(build_argv, NULL, 0, 15000, &build) != 0) {
        result->stage = "io";
        return -1;
    }

    result->exit_code = build.exit_code;
    result->ok = build.exit_code == 0;
    result->diagnostics = xstrdup(build.err);
    if (build.exit_code == 3) {
        result->stage = "backend";
    } else if (build.exit_code != 0) {
        result->stage = "compilacao";
    }
    proc_result_free(&build);

    struct stat st;
    if (stat(result->c_path, &st) == 0 && S_ISREG(st.st_mode)) {
        size_t c_len = 0;
        char *c_data = read_file(result->c_path, &c_len);
        if (c_data) {
            result->c_text = c_data;
        }
    }
    if (!result->c_text) {
        result->c_text = xstrdup("");
    }
    return 0;
}

static void handle_compile(int fd, const char *body)
{
    char *code = json_get_string_alloc(body, "code");
    char *stdin_data = json_get_string_alloc(body, "stdin");
    char *mode = json_get_string_alloc(body, "mode");
    int run_program = !(mode && !strcmp(mode, "emit"));
    free(mode);
    if (!code) {
        code = xstrdup("");
    }
    if (!stdin_data) {
        stdin_data = xstrdup("");
    }

    BuildResult build;
    if (build_source(code, &build) != 0) {
        free(code);
        free(stdin_data);
        http_error(fd, 500, "Não foi possível preparar a compilação.");
        build_result_free(&build);
        return;
    }
    free(code);

    int ok = build.ok;
    const char *stage = build.stage;
    int exit_code = -1;
    int timed_out = 0;
    char *program_out = xstrdup("");
    char *program_err = xstrdup("");

    if (ok) {
        stage = run_program ? "execucao" : "geracao";
        if (run_program) {
            char *run_argv[] = {build.exe_path, NULL};
            ProcResult run;
            if (proc_run_capture(run_argv, stdin_data, strlen(stdin_data),
                                 RUN_TIMEOUT_MS, &run) == 0) {
                free(program_out);
                free(program_err);
                program_out = run.out;
                program_err = run.err;
                exit_code = run.exit_code;
                timed_out = run.timed_out;
                run.out = NULL;
                run.err = NULL;
                proc_result_free(&run);
            }
        } else {
            exit_code = 0;
        }
    }
    free(stdin_data);

    char *esc_diag = json_escape_alloc(build.diagnostics);
    char *esc_c = json_escape_alloc(build.c_text);
    char *esc_out = json_escape_alloc(program_out);
    char *esc_err = json_escape_alloc(program_err);

    StrBuf response;
    sb_init(&response);
    sb_printf(&response,
              "{\"ok\":%s,\"stage\":\"%s\",\"diagnostics\":\"%s\","
              "\"c\":\"%s\",\"stdout\":\"%s\",\"stderr\":\"%s\","
              "\"exitCode\":%d,\"timedOut\":%s}",
              ok ? "true" : "false", stage, esc_diag, esc_c, esc_out, esc_err,
              exit_code, timed_out ? "true" : "false");

    free(esc_diag);
    free(esc_c);
    free(esc_out);
    free(esc_err);
    free(program_out);
    free(program_err);
    build_result_free(&build);

    http_respond(fd, 200, "application/json; charset=utf-8", sb_cstr(&response),
                 response.len);
    sb_free(&response);
}

static void handle_session_create(int fd, const char *body)
{
    char *code = json_get_string_alloc(body, "code");
    if (!code) {
        code = xstrdup("");
    }

    BuildResult build;
    if (build_source(code, &build) != 0) {
        free(code);
        build_result_free(&build);
        http_error(fd, 500, "Não foi possível preparar a sessão.");
        return;
    }
    free(code);

    if (!build.ok) {
        char *esc_diag = json_escape_alloc(build.diagnostics);
        char *esc_c = json_escape_alloc(build.c_text);
        StrBuf response;
        sb_init(&response);
        sb_printf(&response,
                  "{\"ok\":false,\"stage\":\"%s\",\"diagnostics\":\"%s\","
                  "\"c\":\"%s\"}",
                  build.stage, esc_diag, esc_c);
        free(esc_diag);
        free(esc_c);
        build_result_free(&build);
        http_respond(fd, 200, "application/json; charset=utf-8",
                     sb_cstr(&response), response.len);
        sb_free(&response);
        return;
    }

    char *fifo_path = path_join(build.dir, "in.fifo");
    if (mkfifo(fifo_path, 0600) != 0) {
        free(fifo_path);
        build_result_free(&build);
        http_error(fd, 500, "Não foi possível criar o canal de entrada.");
        return;
    }
    free(fifo_path);

    const char *id = build.dir + strlen(SESSION_PREFIX);
    char *esc_diag = json_escape_alloc(build.diagnostics);
    char *esc_c = json_escape_alloc(build.c_text);

    StrBuf response;
    sb_init(&response);
    sb_printf(&response,
              "{\"ok\":true,\"id\":\"%s\",\"stage\":\"pronto\","
              "\"diagnostics\":\"%s\",\"c\":\"%s\"}",
              id, esc_diag, esc_c);

    free(esc_diag);
    free(esc_c);
    free(build.ln_path);
    free(build.c_path);
    free(build.exe_path);
    free(build.diagnostics);
    free(build.c_text);
    free(build.dir);

    http_respond(fd, 200, "application/json; charset=utf-8", sb_cstr(&response),
                 response.len);
    sb_free(&response);
}

static int sse_send(int fd, const char *event, const char *json)
{
    char header[64];
    int header_len = snprintf(header, sizeof(header), "event: %s\ndata: ", event);
    if (send_all(fd, header, (size_t)header_len) != 0) {
        return -1;
    }
    if (send_all(fd, json, strlen(json)) != 0) {
        return -1;
    }
    return send_all(fd, "\n\n", 2);
}

static int sse_send_output(int fd, const char *text, size_t len)
{
    if (len == 0) {
        return 0;
    }
    char *copy = xstrndup(text, len);
    char *escaped = json_escape_alloc(copy);
    free(copy);
    StrBuf json;
    sb_init(&json);
    sb_printf(&json, "{\"text\":\"%s\"}", escaped);
    free(escaped);
    int result = sse_send(fd, "out", sb_cstr(&json));
    sb_free(&json);
    return result;
}

static void handle_session_stream(int fd, const char *id)
{
    char dir[512];
    char fifo_path[512];
    char exe_path[512];
    char stop_path[512];
    char lock_path[512];
    if (session_paths(id, dir, sizeof(dir), fifo_path, sizeof(fifo_path),
                      exe_path, sizeof(exe_path), stop_path,
                      sizeof(stop_path), lock_path, sizeof(lock_path)) != 0) {
        http_error(fd, 404, "Sessão inválida.");
        return;
    }

    struct stat st;
    if (stat(dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        http_error(fd, 404, "Sessão não encontrada ou já encerrada.");
        return;
    }

    int lock = open(lock_path, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (lock < 0) {
        http_error(fd, 409, "A sessão já está em execução.");
        return;
    }
    close(lock);

    int fifo = open(fifo_path, O_RDWR | O_NONBLOCK);
    if (fifo < 0) {
        unlink(lock_path);
        http_error(fd, 500, "Não foi possível abrir o canal de entrada.");
        return;
    }

    /* O programa roda sob um pseudo-terminal para que a biblioteca C use
     * buffer de linha: os prompts aparecem imediatamente, sem esperar o
     * processo terminar. O terminal é colocado em modo cru para não
     * duplicar o eco das entradas (o editor já ecoa as linhas digitadas). */
    int master = posix_openpt(O_RDWR | O_NOCTTY);
    if (master < 0 || grantpt(master) != 0 || unlockpt(master) != 0) {
        if (master >= 0) {
            close(master);
        }
        close(fifo);
        unlink(lock_path);
        http_error(fd, 500, "Não foi possível criar o terminal do processo.");
        return;
    }
    const char *slave_name = ptsname(master);
    int slave = slave_name ? open(slave_name, O_RDWR | O_NOCTTY) : -1;
    if (slave < 0) {
        close(master);
        close(fifo);
        unlink(lock_path);
        http_error(fd, 500, "Não foi possível abrir o terminal do processo.");
        return;
    }

    struct termios tio;
    if (tcgetattr(slave, &tio) == 0) {
        tio.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ISIG |
                                    IEXTEN);
        tio.c_iflag &= (tcflag_t) ~(IXON | ICRNL | INLCR | IGNCR | BRKINT |
                                    INPCK | ISTRIP);
        tio.c_oflag &= (tcflag_t) ~OPOST;
        tio.c_cflag |= CS8;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 0;
        tcsetattr(slave, TCSANOW, &tio);
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(master);
        close(slave);
        close(fifo);
        unlink(lock_path);
        http_error(fd, 500, "Não foi possível iniciar o processo.");
        return;
    }
    if (pid == 0) {
        setsid();
        ioctl(slave, TIOCSCTTY, 0);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave > STDERR_FILENO) {
            close(slave);
        }
        close(master);
        close(fifo);
        execl(exe_path, exe_path, (char *)NULL);
        _exit(127);
    }

    close(slave);
    int flags = fcntl(master, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(master, F_SETFL, flags | O_NONBLOCK);
    }

    StrBuf pending;
    sb_init(&pending);
    int stdin_open = 1;
    int out_open = 1;
    int stopped = 0;
    int timed_out = 0;
    int client_gone = 0;
    int exit_code = -1;
    int exit_signal = 0;
    long long deadline = now_ms() + SESSION_TIMEOUT_MS;

    const char *headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/event-stream; charset=utf-8\r\n"
        "Cache-Control: no-cache\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n";
    if (send_all(fd, headers, strlen(headers)) != 0) {
        client_gone = 1;
        kill(pid, SIGKILL);
    }

    while (out_open && !client_gone) {
        struct pollfd fds[4];
        int nfds = 0;
        int out_index = -1;
        int fifo_index = -1;
        int stdin_index = -1;
        int client_index = -1;

        fds[nfds].fd = master;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        out_index = nfds++;

        fds[nfds].fd = fifo;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        fifo_index = nfds++;

        if (stdin_open && pending.len > 0) {
            fds[nfds].fd = master;
            fds[nfds].events = POLLOUT;
            fds[nfds].revents = 0;
            stdin_index = nfds++;
        }

        fds[nfds].fd = fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        client_index = nfds++;

        int ready = poll(fds, (nfds_t)nfds, 200);
        if (ready < 0 && errno != EINTR) {
            break;
        }

        if (access(stop_path, F_OK) == 0) {
            stopped = 1;
            break;
        }
        if (now_ms() > deadline) {
            timed_out = 1;
            break;
        }

        if (ready <= 0) {
            continue;
        }

        if (client_index >= 0 && fds[client_index].revents) {
            char discard[64];
            ssize_t n = read(fd, discard, sizeof(discard));
            if (n == 0) {
                client_gone = 1;
                break;
            }
        }

        if (fifo_index >= 0 && (fds[fifo_index].revents & POLLIN)) {
            char buffer[1024];
            ssize_t n = read(fifo, buffer, sizeof(buffer));
            if (n > 0) {
                sb_write(&pending, buffer, (size_t)n);
            }
        }

        if (stdin_index >= 0 && (fds[stdin_index].revents & POLLOUT)) {
            ssize_t n = write(master, pending.data, pending.len);
            if (n > 0) {
                memmove(pending.data, pending.data + n,
                        pending.len - (size_t)n);
                pending.len -= (size_t)n;
                pending.data[pending.len] = '\0';
            } else if (n < 0 && errno != EAGAIN && errno != EINTR) {
                stdin_open = 0;
                pending.len = 0;
            }
        }

        if (out_index >= 0 && (fds[out_index].revents & (POLLIN | POLLHUP))) {
            char buffer[4096];
            for (;;) {
                ssize_t n = read(master, buffer, sizeof(buffer));
                if (n > 0) {
                    if (sse_send_output(fd, buffer, (size_t)n) != 0) {
                        client_gone = 1;
                        break;
                    }
                } else if (n < 0 && errno == EINTR) {
                    continue;
                } else {
                    if (n == 0 || (n < 0 && errno == EIO)) {
                        out_open = 0;
                    }
                    break;
                }
            }
        }
    }

    if (stdin_open) {
        stdin_open = 0;
    }
    sb_free(&pending);

    if (stopped || timed_out || client_gone) {
        kill(pid, SIGKILL);
    }

    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
        /* repete */
    }
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        exit_signal = WTERMSIG(status);
        exit_code = -1;
    }

    if (!client_gone) {
        char buffer[4096];
        for (;;) {
            ssize_t n = read(master, buffer, sizeof(buffer));
            if (n > 0) {
                if (sse_send_output(fd, buffer, (size_t)n) != 0) {
                    break;
                }
            } else if (n < 0 && errno == EINTR) {
                continue;
            } else {
                break;
            }
        }
    }

    close(master);
    close(fifo);

    if (!client_gone) {
        char payload[160];
        snprintf(payload, sizeof(payload),
                 "{\"code\":%d,\"signal\":%d,\"timedOut\":%s,\"stopped\":%s}",
                 exit_code, exit_signal, timed_out ? "true" : "false",
                 stopped ? "true" : "false");
        sse_send(fd, "exit", payload);
    }

    remove_session_files(dir);
}

static void handle_session_input(int fd, const char *id, const char *body)
{
    char *data = json_get_string_alloc(body, "data");
    if (!data) {
        http_error(fd, 400, "Campo 'data' ausente.");
        return;
    }
    if (strlen(data) > MAX_INPUT_BYTES) {
        free(data);
        http_error(fd, 413, "Entrada grande demais.");
        return;
    }

    char dir[512];
    char fifo_path[512];
    char exe_path[512];
    char stop_path[512];
    char lock_path[512];
    if (session_paths(id, dir, sizeof(dir), fifo_path, sizeof(fifo_path),
                      exe_path, sizeof(exe_path), stop_path,
                      sizeof(stop_path), lock_path, sizeof(lock_path)) != 0) {
        free(data);
        http_error(fd, 404, "Sessão inválida.");
        return;
    }

    int fifo = open(fifo_path, O_WRONLY | O_NONBLOCK);
    if (fifo < 0) {
        int saved = errno;
        free(data);
        if (saved == ENXIO || saved == ENOENT) {
            http_error(fd, 409,
                       "O terminal não está ativo. Execute o programa de novo.");
        } else {
            http_error(fd, 500, "Falha ao enviar entrada.");
        }
        return;
    }

    size_t len = strlen(data);
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fifo, data + sent, len - sent);
        if (n > 0) {
            sent += (size_t)n;
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    close(fifo);
    free(data);

    if (sent < len) {
        http_error(fd, 409, "O terminal não aceitou toda a entrada.");
        return;
    }
    http_text(fd, 200, "application/json; charset=utf-8", "{\"ok\":true}");
}

static void handle_session_stop(int fd, const char *id)
{
    char dir[512];
    char fifo_path[512];
    char exe_path[512];
    char stop_path[512];
    char lock_path[512];
    if (session_paths(id, dir, sizeof(dir), fifo_path, sizeof(fifo_path),
                      exe_path, sizeof(exe_path), stop_path,
                      sizeof(stop_path), lock_path, sizeof(lock_path)) != 0) {
        http_error(fd, 404, "Sessão inválida.");
        return;
    }

    struct stat st;
    if (stat(dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        http_error(fd, 404, "Sessão não encontrada ou já encerrada.");
        return;
    }

    int stop = open(stop_path, O_CREAT | O_WRONLY, 0600);
    if (stop < 0) {
        http_error(fd, 500, "Falha ao interromper a sessão.");
        return;
    }
    close(stop);
    http_text(fd, 200, "application/json; charset=utf-8", "{\"ok\":true}");
}

/* ---------- Roteamento ---------- */

static void handle_connection(int fd, const ServerOptions *options)
{
    char *method = NULL;
    char *path = NULL;
    char *body = NULL;
    size_t body_len = 0;

    if (read_http_request(fd, &method, &path, &body, &body_len) != 0) {
        http_error(fd, 400, "Requisição HTTP inválida.");
        return;
    }

    if (!strcmp(method, "OPTIONS")) {
        http_respond(fd, 200, "text/plain; charset=utf-8", "", 0);
    } else if (!strcmp(method, "GET") && (!strcmp(path, "/") ||
                                           !strcmp(path, "/index.html"))) {
        serve_file(fd, options->editor, "text/html; charset=utf-8");
    } else if (!strcmp(method, "GET") && !strcmp(path, "/exemplo.ln")) {
        serve_file(fd, options->example, "text/plain; charset=utf-8");
    } else if (!strcmp(method, "GET") &&
               (!strcmp(path, "/curso") || !strcmp(path, "/curso/") ||
                !strcmp(path, "/curso/index.html"))) {
        serve_file(fd, "curso/index.html", "text/html; charset=utf-8");
    } else if (!strcmp(method, "GET") && !strcmp(path, "/curso/styles.css")) {
        serve_file(fd, "curso/styles.css", "text/css; charset=utf-8");
    } else if (!strcmp(method, "GET") && !strcmp(path, "/curso/app.js")) {
        serve_file(fd, "curso/app.js", "text/javascript; charset=utf-8");
    } else if (!strcmp(method, "GET") && !strcmp(path, "/api/health")) {
        http_text(fd, 200, "application/json; charset=utf-8",
                  "{\"ok\":true,\"name\":\"docscript\"}");
    } else if (!strcmp(method, "POST") && !strcmp(path, "/api/compile")) {
        (void)body_len;
        handle_compile(fd, body);
    } else if (!strcmp(method, "POST") && !strcmp(path, "/api/session")) {
        (void)body_len;
        handle_session_create(fd, body);
    } else if (!strncmp(path, "/api/session/", 13)) {
        const char *rest = path + 13;
        const char *slash = strchr(rest, '/');
        if (!slash) {
            http_error(fd, 404, "Rota de sessão inválida.");
        } else {
            size_t id_len = (size_t)(slash - rest);
            char id[32];
            if (id_len >= sizeof(id)) {
                http_error(fd, 404, "Identificador de sessão inválido.");
            } else {
                memcpy(id, rest, id_len);
                id[id_len] = '\0';
                const char *action = slash + 1;
                if (!strcmp(method, "GET") && !strcmp(action, "stream")) {
                    handle_session_stream(fd, id);
                } else if (!strcmp(method, "POST") &&
                           !strcmp(action, "input")) {
                    handle_session_input(fd, id, body);
                } else if (!strcmp(method, "POST") &&
                           !strcmp(action, "stop")) {
                    handle_session_stop(fd, id);
                } else {
                    http_error(fd, 404, "Ação de sessão desconhecida.");
                }
            }
        }
    } else if (!strcmp(path, "/api/compile")) {
        http_error(fd, 405, "Use POST em /api/compile.");
    } else {
        http_error(fd, 404, "Rota não encontrada.");
    }

    free(method);
    free(path);
    free(body);
}

int server_main(int argc, char **argv)
{
    ServerOptions options;
    options.port = 8080;
    options.host = "127.0.0.1";
    options.editor = "editor.html";
    options.example = "exemplo.ln";

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--port") && i + 1 < argc) {
            options.port = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--host") && i + 1 < argc) {
            options.host = argv[++i];
        } else if (!strcmp(argv[i], "--editor") && i + 1 < argc) {
            options.editor = argv[++i];
        } else if (!strcmp(argv[i], "--example") && i + 1 < argc) {
            options.example = argv[++i];
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf("Uso: docscript serve [--port N] [--host H] "
                   "[--editor arquivo.html] [--example arquivo.ln]\n");
            return 0;
        } else {
            fprintf(stderr, "docscript serve: opção desconhecida '%s'.\n",
                    argv[i]);
            return 2;
        }
    }

    resolve_self(argv[0], g_self, sizeof(g_self));
    cleanup_stale_sessions();

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        fprintf(stderr, "docscript serve: falha ao criar socket: %s\n",
                strerror(errno));
        return 1;
    }
    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)options.port);
    if (inet_pton(AF_INET, options.host, &address.sin_addr) != 1) {
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) != 0) {
        fprintf(stderr, "docscript serve: falha ao ligar em %s:%d: %s\n",
                options.host, options.port, strerror(errno));
        close(listen_fd);
        return 1;
    }
    if (listen(listen_fd, 16) != 0) {
        fprintf(stderr, "docscript serve: falha ao escutar: %s\n",
                strerror(errno));
        close(listen_fd);
        return 1;
    }

    socklen_t address_len = sizeof(address);
    if (getsockname(listen_fd, (struct sockaddr *)&address, &address_len) == 0) {
        options.port = ntohs(address.sin_port);
    }

    printf("DOC Script: editor disponível em http://%s:%d/\n", options.host,
           options.port);
    fflush(stdout);

    for (;;) {
        int client = accept(listen_fd, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        pid_t pid = fork();
        if (pid == 0) {
            close(listen_fd);
            /* O processo servidor ignora SIGCHLD para não acumular zumbis;
             * o filho precisa do comportamento padrão para poder usar
             * waitpid() ao compilar e executar o programa do usuário. */
            signal(SIGCHLD, SIG_DFL);
            handle_connection(client, &options);
            close(client);
            _exit(0);
        }
        close(client);
    }

    close(listen_fd);
    return 0;
}
