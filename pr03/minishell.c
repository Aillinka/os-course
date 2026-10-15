#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#define MAX_ARGS 128
#define MAX_CMDS 16
#define MAX_TOKENS 256

typedef struct {
    char *args[MAX_ARGS];
    char *input;
    char *output;
} Command;

static volatile sig_atomic_t child_done = 0;

static void sigchld_handler(int sig) {
    (void)sig;
    child_done = 1;
}

static void reap_children(void) {
    int saved_errno = errno;
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[background process %ld finished]\n", (long)pid);
    }

    child_done = 0;
    errno = saved_errno;
}

static int tokenize(char *line, char **tokens) {
    static char storage[MAX_TOKENS][4096];
    int count = 0;
    char *p = line;

    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        if (count >= MAX_TOKENS - 1) {
            fprintf(stderr, "Too many tokens\n");
            return -1;
        }

        char *dest = storage[count];
        size_t length = 0;

        if (strchr("|<>&", *p)) {
            dest[length++] = *p++;
        } else {
            char quote = 0;

            while (*p) {
                if (!quote && isspace((unsigned char)*p)) break;
                if (!quote && strchr("|<>&", *p)) break;

                if (*p == '\'' || *p == '"') {
                    if (!quote) {
                        quote = *p++;
                        continue;
                    }
                    if (quote == *p) {
                        quote = 0;
                        p++;
                        continue;
                    }
                }

                if (length >= 4095) {
                    fprintf(stderr, "Token too long\n");
                    return -1;
                }

                dest[length++] = *p++;
            }

            if (quote) {
                fprintf(stderr, "Unclosed quote\n");
                return -1;
            }
        }

        dest[length] = '\0';
        tokens[count++] = dest;
    }

    tokens[count] = NULL;
    return count;
}

static int parse_commands(
    char **tokens, int count,
    Command *cmds, int *background
) {
    memset(cmds, 0, sizeof(Command) * MAX_CMDS);
    *background = 0;

    if (count == 0) return 0;

    if (strcmp(tokens[count - 1], "&") == 0) {
        *background = 1;
        count--;
    }

    int ci = 0;
    int ai = 0;

    for (int i = 0; i < count; i++) {
        char *t = tokens[i];

        if (strcmp(t, "|") == 0) {
            if (ai == 0 || ci >= MAX_CMDS - 1) return -1;
            cmds[ci].args[ai] = NULL;
            ci++;
            ai = 0;
        } else if (strcmp(t, "<") == 0 || strcmp(t, ">") == 0) {
            if (++i >= count) return -1;

            if (strcmp(t, "<") == 0)
                cmds[ci].input = tokens[i];
            else
                cmds[ci].output = tokens[i];
        } else if (strcmp(t, "&") == 0) {
            return -1;
        } else {
            if (ai >= MAX_ARGS - 1) return -1;
            cmds[ci].args[ai++] = t;
        }
    }

    if (ai == 0) return -1;
    cmds[ci].args[ai] = NULL;

    return ci + 1;
}

static void redirect_file(const char *path, int target, int flags) {
    int fd = open(path, flags, 0644);

    if (fd < 0) {
        perror(path);
        _exit(1);
    }

    if (dup2(fd, target) < 0) {
        perror("dup2");
        _exit(1);
    }

    close(fd);
}

static void execute_pipeline(
    Command *cmds, int n, int background
) {
    pid_t pids[MAX_CMDS];
    int prev_read = -1;
    int started = 0;

    for (int i = 0; i < n; i++) {
        int pipefd[2] = {-1, -1};

        if (i < n - 1 && pipe(pipefd) < 0) {
            perror("pipe");
            break;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            if (pipefd[0] >= 0) close(pipefd[0]);
            if (pipefd[1] >= 0) close(pipefd[1]);
            break;
        }

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);

            if (prev_read >= 0) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }

            if (pipefd[1] >= 0) {
                dup2(pipefd[1], STDOUT_FILENO);
            }

            if (pipefd[0] >= 0) close(pipefd[0]);
            if (pipefd[1] >= 0) close(pipefd[1]);

            if (cmds[i].input)
                redirect_file(
                    cmds[i].input,
                    STDIN_FILENO,
                    O_RDONLY
                );

            if (cmds[i].output)
                redirect_file(
                    cmds[i].output,
                    STDOUT_FILENO,
                    O_WRONLY | O_CREAT | O_TRUNC
                );

            execvp(cmds[i].args[0], cmds[i].args);

            perror(cmds[i].args[0]);
            _exit(127);
        }

        pids[started++] = pid;

        if (prev_read >= 0) close(prev_read);
        if (pipefd[1] >= 0) close(pipefd[1]);

        prev_read = pipefd[0];
    }

    if (prev_read >= 0) close(prev_read);

    if (background) {
        printf("[background started]");
        for (int i = 0; i < started; i++)
            printf(" %ld", (long)pids[i]);
        printf("\n");
    } else {
        for (int i = 0; i < started; i++) {
            int status;
            while (waitpid(pids[i], &status, 0) < 0) {
                if (errno != EINTR) {
                    perror("waitpid");
                    break;
                }
            }
        }
    }
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    struct sigaction ignore_int = {0};
    ignore_int.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_int, NULL);

    char *line = NULL;
    size_t capacity = 0;

    while (1) {
        if (child_done) reap_children();

        printf("mini-shell> ");
        fflush(stdout);

        ssize_t length = getline(&line, &capacity, stdin);

        if (length < 0) {
            if (feof(stdin)) break;
            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            }
            perror("getline");
            break;
        }

        char *tokens[MAX_TOKENS];
        int count = tokenize(line, tokens);

        if (count <= 0) continue;

        Command cmds[MAX_CMDS];
        int background;

        int n = parse_commands(tokens, count, cmds, &background);

        if (n < 0) {
            fprintf(stderr, "Invalid command syntax\n");
            continue;
        }

        if (n == 1 && strcmp(cmds[0].args[0], "exit") == 0) {
            break;
        }

        if (n == 1 && strcmp(cmds[0].args[0], "cd") == 0) {
            const char *path = cmds[0].args[1];

            if (!path) path = getenv("HOME");
            if (!path) path = "/";

            if (chdir(path) != 0) perror("cd");
            continue;
        }

        execute_pipeline(cmds, n, background);
    }

    free(line);
    reap_children();
    return 0;
}
