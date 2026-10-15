#include <stdio.h>
#include <pthread.h>

#define THREADS 4
#define ITERATIONS 100000

static volatile int counter = 0;

void *worker(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        counter++;
    }
    return NULL;
}

int main(void) {
    pthread_t threads[THREADS];

    for (int i = 0; i < THREADS; i++)
        pthread_create(&threads[i], NULL, worker, NULL);

    for (int i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);

    printf("Expected: %d\n", THREADS * ITERATIONS);
    printf("Actual: %d\n", counter);
    return 0;
}
