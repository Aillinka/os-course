#include <stdio.h>
#include <pthread.h>

#define THREADS 4
#define ITERATIONS 100000

static int counter = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg) {
    (void)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
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

    pthread_mutex_destroy(&mutex);
    return counter == THREADS * ITERATIONS ? 0 : 1;
}
