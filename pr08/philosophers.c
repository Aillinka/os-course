#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#define N 5

static pthread_mutex_t forks[N];
static sem_t waiter;
static pthread_barrier_t barrier;
static atomic_int stop_flag = 0;
static atomic_int completed = 0;

static int meals[N];
static double max_wait[N];
static const char *mode;

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static void pause_us(long us) {
    struct timespec t = {
        .tv_sec = us / 1000000,
        .tv_nsec = (us % 1000000) * 1000
    };
    nanosleep(&t, NULL);
}

static void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    pthread_barrier_wait(&barrier);

    double end = now() + 10.0;

    while (now() < end && !atomic_load(&stop_flag)) {
        double start = now();
        int a = left, b = right;

        if (!strcmp(mode, "ordered") && a > b) {
            int tmp = a;
            a = b;
            b = tmp;
        }

        if (!strcmp(mode, "waiter"))
            sem_wait(&waiter);

        if (!strcmp(mode, "trylock")) {
            if (pthread_mutex_trylock(&forks[left]) != 0) {
                sched_yield();
                continue;
            }

            if (pthread_mutex_trylock(&forks[right]) != 0) {
                pthread_mutex_unlock(&forks[left]);
                sched_yield();
                continue;
            }
        } else {
            pthread_mutex_lock(&forks[a]);

            if (!strcmp(mode, "naive"))
                pthread_barrier_wait(&barrier);

            pthread_mutex_lock(&forks[b]);
        }

        double waited = now() - start;
        if (waited > max_wait[id])
            max_wait[id] = waited;

        meals[id]++;
        atomic_fetch_add(&completed, 1);

        pause_us(100);

        pthread_mutex_unlock(&forks[b]);
        pthread_mutex_unlock(&forks[a]);

        if (!strcmp(mode, "waiter"))
            sem_post(&waiter);

        pause_us(100);
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2 ||
        (strcmp(argv[1], "naive") &&
         strcmp(argv[1], "ordered") &&
         strcmp(argv[1], "trylock") &&
         strcmp(argv[1], "waiter"))) {
        fprintf(stderr,
            "Usage: %s naive|ordered|trylock|waiter\n",
            argv[0]);
        return 1;
    }

    mode = argv[1];

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&forks[i], NULL);

    sem_init(&waiter, 0, N - 1);
    pthread_barrier_init(&barrier, NULL, N);

    pthread_t threads[N];
    int ids[N] = {0, 1, 2, 3, 4};

    for (int i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);

    if (!strcmp(mode, "naive")) {
        pause_us(2000000);
        printf("DEADLOCK DEMONSTRATION\n");
        printf("All philosophers hold one fork and wait.\n");
        fflush(stdout);
        _Exit(0);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    printf("mode=%s\n", mode);
    printf("philosopher,meals,max_wait_seconds\n");

    for (int i = 0; i < N; i++)
        printf("P%d,%d,%.6f\n",
               i, meals[i], max_wait[i]);

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&forks[i]);

    sem_destroy(&waiter);
    pthread_barrier_destroy(&barrier);

    return 0;
}
