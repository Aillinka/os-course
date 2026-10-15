#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define READERS 8
#define WRITERS 2
#define OPERATIONS 1000

static pthread_mutex_t resource = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t reader_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t turnstile = PTHREAD_MUTEX_INITIALIZER;

static int readers = 0;
static int data = 0;

static atomic_int active_readers = 0;
static atomic_int active_writers = 0;
static atomic_int violations = 0;

static int fair_mode = 0;
static double writer_wait[WRITERS] = {0};

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void reader_enter(void) {
    if (fair_mode) {
        pthread_mutex_lock(&turnstile);
        pthread_mutex_unlock(&turnstile);
    }

    pthread_mutex_lock(&reader_mutex);
    readers++;
    if (readers == 1)
        pthread_mutex_lock(&resource);
    pthread_mutex_unlock(&reader_mutex);
}

static void reader_leave(void) {
    pthread_mutex_lock(&reader_mutex);
    readers--;
    if (readers == 0)
        pthread_mutex_unlock(&resource);
    pthread_mutex_unlock(&reader_mutex);
}

static void *reader_thread(void *arg) {
    (void)arg;

    for (int i = 0; i < OPERATIONS; i++) {
        reader_enter();

        atomic_fetch_add(&active_readers, 1);

        if (atomic_load(&active_writers) != 0)
            atomic_fetch_add(&violations, 1);

        int snapshot = data;
        (void)snapshot;

        atomic_fetch_sub(&active_readers, 1);
        reader_leave();
    }

    return NULL;
}

static void *writer_thread(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < OPERATIONS; i++) {
        double start = now();

        if (fair_mode)
            pthread_mutex_lock(&turnstile);

        pthread_mutex_lock(&resource);

        writer_wait[id] += now() - start;

        atomic_fetch_add(&active_writers, 1);

        if (atomic_load(&active_readers) != 0 ||
            atomic_load(&active_writers) != 1)
            atomic_fetch_add(&violations, 1);

        data++;

        atomic_fetch_sub(&active_writers, 1);
        pthread_mutex_unlock(&resource);

        if (fair_mode)
            pthread_mutex_unlock(&turnstile);
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2 ||
        (strcmp(argv[1], "reader") != 0 &&
         strcmp(argv[1], "fair") != 0)) {
        fprintf(stderr, "Usage: %s reader|fair\n", argv[0]);
        return 1;
    }

    fair_mode = strcmp(argv[1], "fair") == 0;

    pthread_t rt[READERS], wt[WRITERS];
    int ids[WRITERS] = {0, 1};

    double start = now();

    for (int i = 0; i < READERS; i++)
        if (pthread_create(&rt[i], NULL, reader_thread, NULL))
            return 1;

    for (int i = 0; i < WRITERS; i++)
        if (pthread_create(&wt[i], NULL, writer_thread, &ids[i]))
            return 1;

    for (int i = 0; i < READERS; i++)
        pthread_join(rt[i], NULL);

    for (int i = 0; i < WRITERS; i++)
        pthread_join(wt[i], NULL);

    double elapsed = now() - start;
    int expected = WRITERS * OPERATIONS;

    printf("mode=%s\n", argv[1]);
    printf("final_data=%d expected=%d\n", data, expected);
    printf("violations=%d\n", atomic_load(&violations));
    printf("writer_0_wait=%.6f seconds\n", writer_wait[0]);
    printf("writer_1_wait=%.6f seconds\n", writer_wait[1]);
    printf("elapsed=%.6f seconds\n", elapsed);

    return data == expected &&
           atomic_load(&violations) == 0 ? 0 : 1;
}
