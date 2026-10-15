#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define ITERATIONS 10000

static double seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void *worker(void *arg) {
    (void)arg;
    return NULL;
}

int main(void) {
    double start = seconds();

    for (int i = 0; i < ITERATIONS; i++) {
        pthread_t thread;

        if (pthread_create(&thread, NULL, worker, NULL) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            return 1;
        }

        pthread_join(thread, NULL);
    }

    double thread_time = seconds() - start;

    start = seconds();

    for (int i = 0; i < ITERATIONS; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0)
            _exit(0);

        if (waitpid(pid, NULL, 0) < 0) {
            perror("waitpid");
            return 1;
        }
    }

    double process_time = seconds() - start;

    printf("operation,iterations,total_seconds,average_us\n");
    printf("pthread_create,10000,%.6f,%.3f\n",
           thread_time, thread_time * 100);
    printf("fork,10000,%.6f,%.3f\n",
           process_time, process_time * 100);

    return 0;
}
