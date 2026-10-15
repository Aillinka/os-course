#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define READERS 8
#define WRITERS 2
#define OPERATIONS 1000

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t can_read = PTHREAD_COND_INITIALIZER;
static pthread_cond_t can_write = PTHREAD_COND_INITIALIZER;

static int active_readers = 0;
static int active_writer = 0;
static int waiting_writers = 0;
static int violations = 0;
static int data = 0;
static int fair_mode = 0;

static double writer_wait[WRITERS];

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void reader_enter(void) {
    pthread_mutex_lock(&mutex);

    while (active_writer ||
           (fair_mode && waiting_writers > 0)) {
        pthread_cond_wait(&can_read, &mutex);
    }

    active_readers++;
    pthread_mutex_unlock(&mutex);
}

static void reader_leave(void) {
    pthread_mutex_lock(&mutex);

    active_readers--;

    if (active_readers == 0)
        pthread_cond_signal(&can_write);

    pthread_mutex_unlock(&mutex);
}

static void writer_enter(int id) {
    double start = now();

    pthread_mutex_lock(&mutex);
    waiting_writers++;

    while (active_writer || active_readers > 0)
        pthread_cond_wait(&can_write, &mutex);

    waiting_writers--;
    active_writer = 1;

    writer_wait[id] += now() - start;

    pthread_mutex_unlock(&mutex);
}

static void writer_leave(void) {
    pthread_mutex_lock(&mutex);

    active_writer = 0;

    if (fair_mode && waiting_writers > 0)
        pthread_cond_signal(&can_write);
    else
        pthread_cond_broadcast(&can_read);

    pthread_mutex_unlock(&mutex);
}

static void *reader_thread(void *arg) {
    (void)arg;

    for (int i = 0; i < OPERATIONS; i++) {
        reader_enter();

        pthread_mutex_lock(&mutex);

        if (active_writer)
            violations++;

        int snapshot = data;
        (void)snapshot;

        pthread_mutex_unlock(&mutex);

        reader_leave();
    }

    return NULL;
}

static void *writer_thread(void *arg) {
    int id = *(int *)arg;

    for (int i = 0; i < OPERATIONS; i++) {
        writer_enter(id);

        pthread_mutex_lock(&mutex);

        if (active_readers != 0 || !active_writer)
            violations++;

        data++;

        pthread_mutex_unlock(&mutex);

        writer_leave();
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

    pthread_t rt[READERS];
    pthread_t wt[WRITERS];
    int ids[WRITERS] = {0, 1};

    double start = now();

    for (int i = 0; i < READERS; i++) {
        if (pthread_create(&rt[i], NULL,
                           reader_thread, NULL) != 0)
            return 1;
    }

    for (int i = 0; i < WRITERS; i++) {
        if (pthread_create(&wt[i], NULL,
                           writer_thread, &ids[i]) != 0)
            return 1;
    }

    for (int i = 0; i < READERS; i++)
        pthread_join(rt[i], NULL);

    for (int i = 0; i < WRITERS; i++)
        pthread_join(wt[i], NULL);

    printf("mode=%s\n", argv[1]);
    printf("final_data=%d expected=%d\n",
           data, WRITERS * OPERATIONS);
    printf("violations=%d\n", violations);
    printf("writer_0_wait=%.6f seconds\n", writer_wait[0]);
    printf("writer_1_wait=%.6f seconds\n", writer_wait[1]);
    printf("elapsed=%.6f seconds\n", now() - start);

    return data == WRITERS * OPERATIONS &&
           violations == 0 ? 0 : 1;
}
