#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <string.h>

#define ITERATIONS 20000

static atomic_flag spin = ATOMIC_FLAG_INIT;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static uint64_t counter = 0;
static volatile uint64_t sink = 0;

typedef struct {
    int use_spin;
    int long_section;
} Task;

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void lock_spin(void) {
    while (atomic_flag_test_and_set_explicit(
        &spin, memory_order_acquire
    )) {
    }
}

static void unlock_spin(void) {
    atomic_flag_clear_explicit(
        &spin, memory_order_release
    );
}

static void *worker(void *arg) {
    Task *task = arg;

    for (int i = 0; i < ITERATIONS; i++) {
        if (task->use_spin)
            lock_spin();
        else
            pthread_mutex_lock(&mutex);

        counter++;

        if (task->long_section) {
            uint64_t local = counter;
            for (int j = 0; j < 1000; j++)
                local = local * 1664525u + 1013904223u;
            sink = local;
        }

        if (task->use_spin)
            unlock_spin();
        else
            pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(void) {
    printf("lock,threads,section,seconds,correct\n");

    int counts[] = {2, 4, 16};

    for (int type = 0; type < 2; type++) {
        for (int section = 0; section < 2; section++) {
            for (int k = 0; k < 3; k++) {
                int n = counts[k];
                pthread_t threads[16];
                Task task = {type, section};

                counter = 0;
                atomic_flag_clear(&spin);

                double start = now();

                for (int i = 0; i < n; i++) {
                    if (pthread_create(
                        &threads[i], NULL, worker, &task
                    ) != 0) {
                        fprintf(stderr, "pthread_create failed\n");
                        return 1;
                    }
                }

                for (int i = 0; i < n; i++)
                    pthread_join(threads[i], NULL);

                double elapsed = now() - start;
                uint64_t expected = (uint64_t)n * ITERATIONS;

                printf("%s,%d,%s,%.6f,%s\n",
                       type ? "spinlock" : "mutex",
                       n,
                       section ? "long" : "short",
                       elapsed,
                       counter == expected ? "YES" : "NO");

                if (counter != expected)
                    return 1;
            }
        }
    }

    return 0;
}
