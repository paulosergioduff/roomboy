#define _POSIX_C_SOURCE 200809L

#include "process.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "strbuf.h"
#include "util.h"

static long long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

static void close_fd(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

static void drain_fd(int *fd, StrBuf *sb)
{
    if (*fd < 0) {
        return;
    }
    int flags = fcntl(*fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(*fd, F_SETFL, flags & ~O_NONBLOCK);
    }
    char buffer[4096];
    for (;;) {
        ssize_t n = read(*fd, buffer, sizeof(buffer));
        if (n > 0) {
            sb_write(sb, buffer, (size_t)n);
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    close_fd(fd);
}

int proc_run_capture(char *const argv[], const char *stdin_data,
                     size_t stdin_len, int timeout_ms, ProcResult *result)
{
    memset(result, 0, sizeof(*result));
    result->exit_code = -1;

    int in_pipe[2] = {-1, -1};
    int out_pipe[2] = {-1, -1};
    int err_pipe[2] = {-1, -1};

    if (pipe(in_pipe) != 0) {
        return -1;
    }
    if (pipe(out_pipe) != 0) {
        close(in_pipe[0]);
        close(in_pipe[1]);
        return -1;
    }
    if (pipe(err_pipe) != 0) {
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        close(err_pipe[0]);
        close(err_pipe[1]);
        return -1;
    }

    if (pid == 0) {
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(err_pipe[1], STDERR_FILENO);
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        close(err_pipe[0]);
        close(err_pipe[1]);
        execvp(argv[0], argv);
        fprintf(stderr, "docscript: não foi possível executar '%s': %s\n",
                argv[0], strerror(errno));
        _exit(127);
    }

    close(in_pipe[0]);
    in_pipe[0] = -1;
    close(out_pipe[1]);
    out_pipe[1] = -1;
    close(err_pipe[1]);
    err_pipe[1] = -1;

    int flags = fcntl(out_pipe[0], F_GETFL, 0);
    if (flags >= 0) {
        fcntl(out_pipe[0], F_SETFL, flags | O_NONBLOCK);
    }
    flags = fcntl(err_pipe[0], F_GETFL, 0);
    if (flags >= 0) {
        fcntl(err_pipe[0], F_SETFL, flags | O_NONBLOCK);
    }

    StrBuf out;
    StrBuf err;
    sb_init(&out);
    sb_init(&err);

    int stdin_open = stdin_data != NULL && stdin_len > 0;
    if (!stdin_open) {
        close_fd(&in_pipe[1]);
    }
    size_t written = 0;
    int out_open = 1;
    int err_open = 1;
    long long deadline = timeout_ms > 0 ? now_ms() + timeout_ms : 0;

    while (out_open || err_open) {
        struct pollfd fds[3];
        int nfds = 0;
        int out_index = -1;
        int err_index = -1;
        int in_index = -1;

        if (out_open) {
            fds[nfds].fd = out_pipe[0];
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            out_index = nfds++;
        }
        if (err_open) {
            fds[nfds].fd = err_pipe[0];
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            err_index = nfds++;
        }
        if (stdin_open) {
            fds[nfds].fd = in_pipe[1];
            fds[nfds].events = POLLOUT;
            fds[nfds].revents = 0;
            in_index = nfds++;
        }

        int wait = 200;
        if (deadline) {
            long long left = deadline - now_ms();
            if (left <= 0) {
                result->timed_out = 1;
                kill(pid, SIGKILL);
                break;
            }
            if (left < wait) {
                wait = (int)left;
            }
        }

        int ready = poll(fds, (nfds_t)nfds, wait);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (ready == 0) {
            continue;
        }

        if (in_index >= 0 && fds[in_index].revents) {
            ssize_t n = write(in_pipe[1], stdin_data + written,
                              stdin_len - written);
            if (n > 0) {
                written += (size_t)n;
            } else if (n < 0 && errno != EAGAIN && errno != EINTR) {
                close_fd(&in_pipe[1]);
                stdin_open = 0;
            }
            if (written >= stdin_len) {
                close_fd(&in_pipe[1]);
                stdin_open = 0;
            }
        }

        if (out_index >= 0 && fds[out_index].revents) {
            char buffer[4096];
            for (;;) {
                ssize_t n = read(out_pipe[0], buffer, sizeof(buffer));
                if (n > 0) {
                    sb_write(&out, buffer, (size_t)n);
                } else if (n < 0 && errno == EINTR) {
                    continue;
                } else {
                    if (n == 0) {
                        close_fd(&out_pipe[0]);
                        out_open = 0;
                    }
                    break;
                }
            }
        }

        if (err_index >= 0 && fds[err_index].revents) {
            char buffer[4096];
            for (;;) {
                ssize_t n = read(err_pipe[0], buffer, sizeof(buffer));
                if (n > 0) {
                    sb_write(&err, buffer, (size_t)n);
                } else if (n < 0 && errno == EINTR) {
                    continue;
                } else {
                    if (n == 0) {
                        close_fd(&err_pipe[0]);
                        err_open = 0;
                    }
                    break;
                }
            }
        }
    }

    if (result->timed_out) {
        kill(pid, SIGKILL);
    }
    close_fd(&in_pipe[1]);

    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
        /* repete */
    }

    drain_fd(&out_pipe[0], &out);
    drain_fd(&err_pipe[0], &err);

    if (WIFEXITED(status)) {
        result->exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result->signal = WTERMSIG(status);
        result->exit_code = -1;
    }

    result->out = sb_take(&out);
    result->err = sb_take(&err);
    result->out_len = strlen(result->out);
    result->err_len = strlen(result->err);
    return 0;
}

int proc_run_inherit(char *const argv[])
{
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "docscript: não foi possível executar '%s': %s\n",
                argv[0], strerror(errno));
        _exit(127);
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            return -1;
        }
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return -1;
}

void proc_result_free(ProcResult *result)
{
    free(result->out);
    free(result->err);
    result->out = NULL;
    result->err = NULL;
    result->out_len = 0;
    result->err_len = 0;
}
