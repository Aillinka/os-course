
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define CHUNK 4096

typedef struct {
    sem_t ready;
    sem_t done;
    char data[CHUNK];
} SharedData;

double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

void send_data(int fd, char *buffer, size_t size) {
    size_t sent = 0;

    while (sent < size) {
        ssize_t n = write(fd, buffer + sent, size - sent);

        if (n <= 0) {
            perror("write");
            exit(1);
        }

        sent += n;
    }
}

void receive_data(int fd, char *buffer, size_t size) {
    size_t received = 0;

    while (received < size) {
        ssize_t n = read(fd, buffer + received, size - received);

        if (n <= 0) {
            perror("read");
            exit(1);
        }

        received += n;
    }
}

void test_pipe(size_t total, size_t message_size) {
    int fd[2];
    pipe(fd);

    char *buffer = malloc(message_size);
    memset(buffer, 'A', message_size);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[1]);

        size_t received = 0;

        while (received < total) {
            size_t n = message_size;

            if (total - received < n)
                n = total - received;

            receive_data(fd[0], buffer, n);
            received += n;
        }

        close(fd[0]);
        free(buffer);
        exit(0);
    }

    close(fd[0]);

    double start = get_time();

    size_t sent = 0;

    while (sent < total) {
        size_t n = message_size;

        if (total - sent < n)
            n = total - sent;

        send_data(fd[1], buffer, n);
        sent += n;
    }

    close(fd[1]);
    waitpid(pid, NULL, 0);

    double elapsed = get_time() - start;

    printf("PIPE: %.2f MB/s, %.3f us/message\n",
           total / 1048576.0 / elapsed,
           elapsed * 1000000.0 / ((total + message_size - 1) / message_size));

    free(buffer);
}

void test_socket(size_t total, size_t message_size) {
    int fd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    char *buffer = malloc(message_size);
    memset(buffer, 'B', message_size);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);

        size_t received = 0;

        while (received < total) {
            size_t n = message_size;

            if (total - received < n)
                n = total - received;

            receive_data(fd[1], buffer, n);
            received += n;
        }

        close(fd[1]);
        free(buffer);
        exit(0);
    }

    close(fd[1]);

    double start = get_time();

    size_t sent = 0;

    while (sent < total) {
        size_t n = message_size;

        if (total - sent < n)
            n = total - sent;

        send_data(fd[0], buffer, n);
        sent += n;
    }

    close(fd[0]);
    waitpid(pid, NULL, 0);

    double elapsed = get_time() - start;

    printf("SOCKET: %.2f MB/s, %.3f us/message\n",
           total / 1048576.0 / elapsed,
           elapsed * 1000000.0 / ((total + message_size - 1) / message_size));

    free(buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <message_size> <total_MB>\n", argv[0]);
        return 1;
    }

    size_t message_size = strtoull(argv[1], NULL, 10);
    size_t total = strtoull(argv[2], NULL, 10) * 1048576ULL;

    if (message_size == 0 || total == 0) {
        printf("Invalid arguments\n");
        return 1;
    }

    printf("Message size: %zu bytes\n", message_size);
    printf("Total data: %.0f MB\n\n", total / 1048576.0);

    test_pipe(total, message_size);
    test_socket(total, message_size);

    return 0;
}
