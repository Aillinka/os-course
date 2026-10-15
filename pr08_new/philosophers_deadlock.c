
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
#define RUN_SECONDS 10

pthread_mutex_t forks[N];
pthread_barrier_t barrier;
sem_t waiter;

atomic_int running = 1;
int meals[N] = {0};
double max_wait[N] = {0};

const char *mode;

double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

void sleep_ms(long ms) {
    struct timespec t = {
        .tv_sec = ms / 1000,
        .tv_nsec = (ms % 1000) * 1000000L
    };
    nanosleep(&t, NULL);
}

void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % N;

    pthread_barrier_wait(&barrier);

    double end = now() + RUN_SECONDS;

    while (atomic_load(&running) && now() < end) {
        double start = now();

        if (strcmp(mode, "ordered") == 0) {
            int first = left < right ? left : right;
            int second = left < right ? right : left;

            pthread_mutex_lock(&forks[first]);
            pthread_mutex_lock(&forks[second]);

        } else if (strcmp(mode, "trylock") == 0) {
            if (pthread_mutex_trylock(&forks[left]) != 0) {
                sched_yield();
                continue;
            }

            if (pthread_mutex_trylock(&forks[right]) != 0) {
                pthread_mutex_unlock(&forks[left]);
                sched_yield();
                continue;
            }

        } else if (strcmp(mode, "waiter") == 0) {
            sem_wait(&waiter);

            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);

        } else if (strcmp(mode, "naive") == 0) {
            pthread_mutex_lock(&forks[left]);

            // All philosophers take one fork first.
            pthread_barrier_wait(&barrier);

            // Circular wait causes deadlock.
            pthread_mutex_lock(&forks[right]);
        }

        double waited = now() - start;

        if (waited > max_wait[id])
            max_wait[id] = waited;

        meals[id]++;

        sleep_ms(1);

        pthread_mutex_unlock(&forks[right]);
        pthread_mutex_unlock(&forks[left]);

        if (strcmp(mode, "waiter") == 0)
            sem_post(&waiter);

        sleep_ms(1);
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s naive|ordered|trylock|waiter\n", argv[0]);
        return 1;
    }

    mode = argv[1];

    if (strcmp(mode, "naive") != 0 &&
        strcmp(mode, "ordered") != 0 &&
        strcmp(mode, "trylock") != 0 &&
        strcmp(mode, "waiter") != 0) {
        printf("Unknown mode\n");
        return 1;
    }

    pthread_t threads[N];
    int ids[N];

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&forks[i], NULL);

    pthread_barrier_init(&barrier, NULL, N);
    sem_init(&waiter, 0, N - 1);

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
    }


    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    printf("\nMode: %s\n", mode);
    printf("Philosopher | Meals | Max wait (ms)\n");

    for (int i = 0; i < N; i++) {
        printf("P%d          | %d     | %.3f\n",
               i, meals[i], max_wait[i] * 1000.0);
    }

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&forks[i]);

    pthread_barrier_destroy(&barrier);
    sem_destroy(&waiter);

    return 0;
}

