#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ARRAY_SIZE (16 * 1024 * 1024)

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE);

    if (page_size <= 0) {
        printf("Cannot determine page size\n");
        return 1;
    }

    unsigned char *array = malloc(ARRAY_SIZE);

    if (array == NULL) {
        perror("malloc");
        return 1;
    }

    volatile unsigned long sum = 0;
    clock_t start, end;

    for (size_t i = 0; i < ARRAY_SIZE; i += (size_t)page_size) {
        array[i] = 1;
    }

    printf("PR10 C: TLB experiment\n");
    printf("Page size: %ld bytes\n", page_size);
    printf("Array size: %d bytes\n", ARRAY_SIZE);

    start = clock();

    for (int repeat = 0; repeat < 100; repeat++) {
        for (size_t i = 0; i < ARRAY_SIZE; i++) {
            sum += array[i];
        }
    }

    end = clock();

    printf("Sequential access time: %.6f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    start = clock();

    for (int repeat = 0; repeat < 100; repeat++) {
        for (size_t i = 0; i < ARRAY_SIZE; i += (size_t)page_size) {
            sum += array[i];
        }
    }

    end = clock();

    printf("Page-stride access time: %.6f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    printf("Checksum: %lu\n", sum);

    free(array);
    return 0;
}
