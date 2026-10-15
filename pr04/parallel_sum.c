#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <inttypes.h>

#define SIZE 100000000UL

typedef struct {
    const uint32_t *array;
    size_t start;
    size_t end;
    uint64_t result;
} Task;

double seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void *worker(void *arg) {
    Task *task = arg;
    uint64_t sum = 0;

    for (size_t i = task->start; i < task->end; i++)
        sum += task->array[i];

    task->result = sum;
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s THREADS\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    if (n < 1 || n > 16) {
        fprintf(stderr, "Threads must be 1..16\n");
        return 1;
    }

    uint32_t *array = malloc(SIZE * sizeof(uint32_t));

    if (!array) {
        perror("malloc");
        return 1;
    }

    for (size_t i = 0; i < SIZE; i++)
        array[i] = (uint32_t)(i % 100 + 1);

    pthread_t threads[16];
    Task tasks[16];

    double start = seconds();

    for (int i = 0; i < n; i++) {
        tasks[i].array = array;
        tasks[i].start = SIZE * (size_t)i / n;
        tasks[i].end = SIZE * (size_t)(i + 1) / n;
        tasks[i].result = 0;

        if (pthread_create(
            &threads[i], NULL, worker, &tasks[i]
        ) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            free(array);
            return 1;
        }
    }

    uint64_t total = 0;

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
        total += tasks[i].result;
    }

    double elapsed = seconds() - start;

    uint64_t expected = 5050000000ULL;

    if (total != expected) {
        fprintf(stderr, "Incorrect sum!\n");
        free(array);
        return 1;
    }

    printf("%d,%.6f,%" PRIu64 "\n",
           n, elapsed, total);

    free(array);
    return 0;
}
