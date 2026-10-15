#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define ITERATIONS 30000000UL

typedef struct {
    volatile uint64_t value;
} Compact;

typedef struct {
    _Alignas(64) volatile uint64_t value;
    char padding[64];
} Padded;

static Compact compact[2];
static Padded padded[2];

typedef struct {
    int index;
    int use_padding;
} Task;

static double seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void *worker(void *arg) {
    Task *task = arg;
    volatile uint64_t *value;

    if (task->use_padding)
        value = &padded[task->index].value;
    else
        value = &compact[task->index].value;

    for (size_t i = 0; i < ITERATIONS; i++)
        (*value)++;

    return NULL;
}

static void run_test(int use_padding) {
    pthread_t threads[2];
    Task tasks[2] = {
        {0, use_padding},
        {1, use_padding}
    };

    compact[0].value = compact[1].value = 0;
    padded[0].value = padded[1].value = 0;

    double start = seconds();

    for (int i = 0; i < 2; i++) {
        if (pthread_create(
            &threads[i], NULL, worker, &tasks[i]
        ) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < 2; i++)
        pthread_join(threads[i], NULL);

    double elapsed = seconds() - start;

    uint64_t a = use_padding ?
        padded[0].value : compact[0].value;

    uint64_t b = use_padding ?
        padded[1].value : compact[1].value;

    if (a != ITERATIONS || b != ITERATIONS) {
        fprintf(stderr, "Incorrect counters\n");
        exit(1);
    }

    printf("%s,%.6f\n",
           use_padding ? "padded" : "shared",
           elapsed);
}

int main(void) {
    printf("mode,time_seconds\n");

    run_test(0);
    run_test(1);

    return 0;
}
