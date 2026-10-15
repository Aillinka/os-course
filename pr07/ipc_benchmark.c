#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>

#define SLOT_SIZE (1024 * 1024)
#define MQ_CHUNK 4096

typedef struct {
    sem_t empty;
    sem_t full;
    size_t length;
    unsigned char data[SLOT_SIZE];
} Shared;

static volatile sig_atomic_t stopped = 0;
static pid_t child_pid = -1;
static mqd_t queue_fd = (mqd_t)-1;
static Shared *shared = NULL;
static int shm_fd = -1;
static char resource_name[80];

static void on_signal(int sig) {
    (void)sig;
    stopped = 1;
}

static double seconds(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static void cleanup(void) {
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0);
        child_pid = -1;
    }

    if (queue_fd != (mqd_t)-1)
        mq_close(queue_fd);

    if (shared) {
        sem_destroy(&shared->empty);
        sem_destroy(&shared->full);
        munmap(shared, sizeof(*shared));
    }

    if (shm_fd >= 0)
        close(shm_fd);

    if (resource_name[0]) {
        mq_unlink(resource_name);
        shm_unlink(resource_name);
    }
}

static void die(const char *message) {
    perror(message);
    cleanup();
    exit(1);
}

static void fill(unsigned char *p, size_t n) {
    memset(p, 0x5a, n);
}

static int valid(const unsigned char *p, size_t n) {
    for (size_t i = 0; i < n; i++)
        if (p[i] != 0x5a)
            return 0;
    return 1;
}

static int write_all(int fd, const unsigned char *buf, size_t n) {
    size_t done = 0;
    while (done < n && !stopped) {
        ssize_t r = write(fd, buf + done, n - done);
        if (r > 0)
            done += (size_t)r;
        else if (r < 0 && errno == EINTR)
            continue;
        else
            return -1;
    }
    return done == n ? 0 : -1;
}

static int read_all(int fd, unsigned char *buf, size_t n) {
    size_t done = 0;
    while (done < n && !stopped) {
        ssize_t r = read(fd, buf + done, n - done);
        if (r > 0)
            done += (size_t)r;
        else if (r < 0 && errno == EINTR)
            continue;
        else
            return -1;
    }
    return done == n ? 0 : -1;
}

static int sem_wait_retry(sem_t *s) {
    while (sem_wait(s) < 0) {
        if (errno == EINTR && !stopped)
            continue;
        return -1;
    }
    return 0;
}

static void run_stream(int socket_mode, size_t msg,
                       size_t total) {
    int fds[2];

    if (socket_mode) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0)
            die("socketpair");
    } else {
        if (pipe(fds) < 0)
            die("pipe");
    }

    unsigned char *buffer = malloc(msg);
    if (!buffer)
        die("malloc");

    fill(buffer, msg);
    child_pid = fork();
    if (child_pid < 0)
        die("fork");

    if (child_pid == 0) {
        close(fds[1]);
        int ok = 1;
        for (size_t pos = 0; pos < total; pos += msg) {
            size_t n = total - pos < msg ? total - pos : msg;
            if (read_all(fds[0], buffer, n) ||
                !valid(buffer, n)) {
                ok = 0;
                break;
            }
        }
        close(fds[0]);
        free(buffer);
        _exit(ok ? 0 : 1);
    }

    close(fds[0]);
    double start = seconds();

    for (size_t pos = 0; pos < total && !stopped; pos += msg) {
        size_t n = total - pos < msg ? total - pos : msg;
        if (write_all(fds[1], buffer, n))
            break;
    }

    close(fds[1]);
    int status = 0;
    waitpid(child_pid, &status, 0);
    child_pid = -1;

    double elapsed = seconds() - start;
    int ok = !stopped && WIFEXITED(status) &&
             WEXITSTATUS(status) == 0;

    printf("%s,%zu,%zu,%.6f,%.2f,%.3f,%s\n",
           socket_mode ? "unix_socket" : "pipe",
           msg, total, elapsed,
           (total / 1048576.0) / elapsed,
           elapsed * 1e6 / ((total + msg - 1) / msg),
           ok ? "PASS" : "FAIL");

    free(buffer);
    if (!ok)
        exit(1);
}

static void run_queue(size_t msg, size_t total) {
    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MQ_CHUNK;

    queue_fd = mq_open(resource_name,
                       O_CREAT | O_EXCL | O_RDWR,
                       0600, &attr);
    if (queue_fd == (mqd_t)-1)
        die("mq_open");

    unsigned char buffer[MQ_CHUNK];
    fill(buffer, sizeof(buffer));

    child_pid = fork();
    if (child_pid < 0)
        die("fork");

    if (child_pid == 0) {
        int ok = 1;
        size_t received = 0;
        while (received < total && !stopped) {
            ssize_t n = mq_receive(queue_fd,
                                   (char *)buffer,
                                   sizeof(buffer), NULL);
            if (n < 0) {
                if (errno == EINTR)
                    continue;
                ok = 0;
                break;
            }
            if (!valid(buffer, (size_t)n)) {
                ok = 0;
                break;
            }
            received += (size_t)n;
        }
        mq_close(queue_fd);
        _exit(ok && received == total ? 0 : 1);
    }

    double start = seconds();
    int send_ok = 1;

    for (size_t pos = 0; pos < total && !stopped;) {
        size_t remaining_message = msg - (pos % msg);
        size_t n = total - pos;
        if (n > remaining_message)
            n = remaining_message;
        if (n > MQ_CHUNK)
            n = MQ_CHUNK;

        if (mq_send(queue_fd, (char *)buffer, n, 0) < 0) {
            if (errno == EINTR && !stopped)
                continue;
            send_ok = 0;
            break;
        }
        pos += n;
    }

    if (!send_ok || stopped)
        kill(child_pid, SIGTERM);

    int status = 0;
    waitpid(child_pid, &status, 0);
    child_pid = -1;

    double elapsed = seconds() - start;
    int ok = send_ok && !stopped &&
             WIFEXITED(status) && WEXITSTATUS(status) == 0;

    printf("message_queue,%zu,%zu,%.6f,%.2f,%.3f,%s\n",
           msg, total, elapsed,
           (total / 1048576.0) / elapsed,
           elapsed * 1e6 / ((total + msg - 1) / msg),
           ok ? "PASS" : "FAIL");

    mq_close(queue_fd);
    queue_fd = (mqd_t)-1;
    mq_unlink(resource_name);

    if (!ok)
        exit(1);
}

static void run_shared(size_t msg, size_t total) {
    shm_fd = shm_open(resource_name,
                     O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd < 0)
        die("shm_open");

    if (ftruncate(shm_fd, sizeof(Shared)) < 0)
        die("ftruncate");

    shared = mmap(NULL, sizeof(Shared),
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        shared = NULL;
        die("mmap");
    }

    if (sem_init(&shared->empty, 1, 1) ||
        sem_init(&shared->full, 1, 0))
        die("sem_init");

    child_pid = fork();
    if (child_pid < 0)
        die("fork");

    if (child_pid == 0) {
        int ok = 1;
        size_t received = 0;
        while (received < total && !stopped) {
            if (sem_wait_retry(&shared->full)) {
                ok = 0;
                break;
            }
            size_t n = shared->length;
            if (n == 0 || n > SLOT_SIZE ||
                !valid(shared->data, n)) {
                ok = 0;
                break;
            }
            received += n;
            sem_post(&shared->empty);
        }
        _exit(ok && received == total ? 0 : 1);
    }

    double start = seconds();
    int send_ok = 1;

    for (size_t pos = 0; pos < total && !stopped; pos += msg) {
        size_t n = total - pos < msg ? total - pos : msg;
        if (sem_wait_retry(&shared->empty)) {
            send_ok = 0;
            break;
        }
        fill(shared->data, n);
        shared->length = n;
        sem_post(&shared->full);
    }

    if (!send_ok || stopped)
        kill(child_pid, SIGTERM);

    int status = 0;
    waitpid(child_pid, &status, 0);
    child_pid = -1;

    double elapsed = seconds() - start;
    int ok = send_ok && !stopped &&
             WIFEXITED(status) && WEXITSTATUS(status) == 0;

    printf("shared_memory,%zu,%zu,%.6f,%.2f,%.3f,%s\n",
           msg, total, elapsed,
           (total / 1048576.0) / elapsed,
           elapsed * 1e6 / ((total + msg - 1) / msg),
           ok ? "PASS" : "FAIL");

    sem_destroy(&shared->empty);
    sem_destroy(&shared->full);
    munmap(shared, sizeof(*shared));
    shared = NULL;
    close(shm_fd);
    shm_fd = -1;
    shm_unlink(resource_name);

    if (!ok)
        exit(1);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr,
            "Usage: %s pipe|mq|shm|socket 64|4096|1048576 MB\n",
            argv[0]);
        return 1;
    }

    char *end;
    unsigned long size = strtoul(argv[2], &end, 10);
    if (*end || (size != 64 && size != 4096 &&
                 size != 1048576)) {
        fprintf(stderr, "Invalid message size\n");
        return 1;
    }

    unsigned long mb = strtoul(argv[3], &end, 10);
    if (*end || mb == 0 || mb > 1024) {
        fprintf(stderr, "MB must be 1..1024\n");
        return 1;
    }

    size_t total = (size_t)mb * 1048576;

    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    snprintf(resource_name, sizeof(resource_name),
             "/pr07_%ld", (long)getpid());

    printf("method,message_bytes,total_bytes,seconds,"
           "throughput_mib_s,us_per_message,result\n");
    fflush(stdout);

    if (!strcmp(argv[1], "pipe"))
        run_stream(0, size, total);
    else if (!strcmp(argv[1], "socket"))
        run_stream(1, size, total);
    else if (!strcmp(argv[1], "mq"))
        run_queue(size, total);
    else if (!strcmp(argv[1], "shm"))
        run_shared(size, total);
    else {
        fprintf(stderr, "Unknown method\n");
        return 1;
    }

    return 0;
}
