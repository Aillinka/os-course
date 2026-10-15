#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>

#define MB (1024 * 1024)

void show_faults(struct rusage before, struct rusage after) {
    printf("Minor page faults: %ld\n",
           after.ru_minflt - before.ru_minflt);

    printf("Major page faults: %ld\n",
           after.ru_majflt - before.ru_majflt);
}

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    size_t size = 128 * MB;

    printf("System page size: %ld bytes\n", page_size);
    printf("Memory allocation: %zu MB\n", size / MB);

    char *memory = malloc(size);

    if (memory == NULL) {
        perror("malloc");
        return 1;
    }

    struct rusage before, after;

    getrusage(RUSAGE_SELF, &before);

    for (size_t i = 0; i < size; i += page_size) {
        memory[i] = 1;
    }

    getrusage(RUSAGE_SELF, &after);

    printf("\nAfter first memory access:\n");
    show_faults(before, after);

    getrusage(RUSAGE_SELF, &before);

    volatile long sum = 0;

    for (size_t i = 0; i < size; i += page_size) {
        sum += memory[i];
    }

    getrusage(RUSAGE_SELF, &after);

    printf("\nAfter second memory access:\n");
    show_faults(before, after);
    printf("Checksum: %ld\n", sum);

    free(memory);

    printf("\nMemory released successfully.\n");

    return 0;
}
