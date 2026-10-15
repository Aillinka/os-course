#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 1048576
#define MQ_CHUNK 8192

typedef struct {
    sem_t empty;
    sem_t full;
    size_t length;
    char buffer[BUFFER_SIZE];
} SharedData;

static volatile sig_atomic_t stop_requested = 0;
static volatile sig_atomic_t child_pid = -1;

static void handle_sigterm(int sig) {
    (void)sig;
    stop_requested = 1;
}

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static void install_handler(void) {
    struct sigaction sa = {0};
    sa.sa_handler = handle_sigterm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
}

static void stop_child(pid_t pid) {
    if (pid <= 0) return;

    if (stop_requested)
        kill(pid, SIGTERM);

    while (waitpid(pid, NULL, 0) == -1) {
        if (errno == EINTR) {
            if (stop_requested)
                kill(pid, SIGTERM);
            continue;
        }
        break;
    }
}

static void print_result(const char *name,
                         size_t total, size_t messages,
                         double elapsed) {
    if (elapsed <= 0 || messages == 0) return;

    printf("%s: %.2f MB/s, %.3f us/message\n",
           name,
           total / 1048576.0 / elapsed,
           elapsed * 1e6 / messages);
}

static void test_mq(size_t total, size_t message_size) {
    char name[64];
    snprintf(name, sizeof(name), "/pr7_mq_%ld", (long)getpid());

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MQ_CHUNK;

    mqd_t mq = mq_open(name, O_CREAT | O_EXCL | O_RDWR,
                       0600, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return;
    }

    char buffer[MQ_CHUNK];
    memset(buffer, 'M', sizeof(buffer));

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        mq_close(mq);
        mq_unlink(name);
        return;
    }

    if (pid == 0) {
        size_t received = 0;
        while (received < total && !stop_requested) {
            ssize_t n = mq_receive(mq, buffer, sizeof(buffer), NULL);
            if (n < 0) {
                if (errno == EINTR && stop_requested) break;
                if (errno == EINTR) continue;
                break;
            }
            received += (size_t)n;
        }
        mq_close(mq);
        _exit(0);
    }

    child_pid = pid;
    size_t sent = 0, messages = 0;
    double start = now();

    while (sent < total && !stop_requested) {
        size_t n = message_size;
        if (n > MQ_CHUNK) n = MQ_CHUNK;
        if (n > total - sent) n = total - sent;

        if (mq_send(mq, buffer, n, 0) == -1) {
            if (errno == EINTR && stop_requested) break;
            if (errno == EINTR) continue;
            perror("mq_send");
            break;
        }

        sent += n;
        messages++;
    }

    double elapsed = now() - start;
    stop_child(pid);
    child_pid = -1;

    mq_close(mq);
    mq_unlink(name);

    if (!stop_requested && sent == total)
        print_result("MESSAGE QUEUE", sent, messages, elapsed);
}

static void test_shm(size_t total, size_t message_size) {
    char name[64];
    snprintf(name, sizeof(name), "/pr7_shm_%ld", (long)getpid());

    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
        perror("shm_open");
        return;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        close(fd);
        shm_unlink(name);
        return;
    }

    SharedData *shared = mmap(NULL, sizeof(SharedData),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        close(fd);
        shm_unlink(name);
        return;
    }

    if (sem_init(&shared->empty, 1, 1) == -1 ||
        sem_init(&shared->full, 1, 0) == -1) {
        perror("sem_init");
        munmap(shared, sizeof(SharedData));
        close(fd);
        shm_unlink(name);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        sem_destroy(&shared->empty);
        sem_destroy(&shared->full);
        munmap(shared, sizeof(SharedData));
        close(fd);
        shm_unlink(name);
        return;
    }

    if (pid == 0) {
        size_t received = 0;

        while (received < total && !stop_requested) {
            if (sem_wait(&shared->full) == -1) {
                if (errno == EINTR && stop_requested) break;
                if (errno == EINTR) continue;
                break;
            }

            received += shared->length;
            sem_post(&shared->empty);
        }

        munmap(shared, sizeof(SharedData));
        close(fd);
        _exit(0);
    }

    child_pid = pid;
    size_t sent = 0, messages = 0;
    double start = now();

    while (sent < total && !stop_requested) {
        if (sem_wait(&shared->empty) == -1) {
            if (errno == EINTR && stop_requested) break;
            if (errno == EINTR) continue;
            break;
        }

        size_t n = message_size;
        if (n > BUFFER_SIZE) n = BUFFER_SIZE;
        if (n > total - sent) n = total - sent;

        memset(shared->buffer, 'S', n);
        shared->length = n;

        sem_post(&shared->full);
        sent += n;
        messages++;
    }

    double elapsed = now() - start;
    stop_child(pid);
    child_pid = -1;

    sem_destroy(&shared->empty);
    sem_destroy(&shared->full);
    munmap(shared, sizeof(SharedData));
    close(fd);
    shm_unlink(name);

    if (!stop_requested && sent == total)
        print_result("SHARED MEMORY", sent, messages, elapsed);
}

int main(int argc, char **argv) {
    install_handler();

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <message_size> <total_MB>\n",
                argv[0]);
        return 1;
    }

    size_t message_size = strtoull(argv[1], NULL, 10);
    size_t total_mb = strtoull(argv[2], NULL, 10);

    if (message_size == 0 || total_mb == 0 ||
        message_size > BUFFER_SIZE) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    size_t total = total_mb * 1048576ULL;

    test_mq(total, message_size);
    if (!stop_requested)
        test_shm(total, message_size);

    return stop_requested ? 130 : 0;
}
