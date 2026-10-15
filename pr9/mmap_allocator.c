
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/resource.h>

#define REGION_SIZE (1024 * 1024)
#define MAX_BLOCKS 4096
#define MAX_ITEMS 2048

typedef struct {
    size_t offset;
    size_t size;
    int free;
} Block;

static unsigned char *region = NULL;
static Block blocks[MAX_BLOCKS];
static size_t block_count = 0;
static int strategy = 0; /* 0 = First Fit, 1 = Best Fit */

static size_t align_size(size_t size) {
    return (size + 7U) & ~(size_t)7U;
}

static int init_allocator(void) {
    region = mmap(NULL, REGION_SIZE, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (region == MAP_FAILED) {
        perror("mmap");
        region = NULL;
        return 0;
    }

    blocks[0] = (Block){0, REGION_SIZE, 1};
    block_count = 1;
    return 1;
}

static void merge_free(void) {
    size_t i = 0;

    while (i + 1 < block_count) {
        if (blocks[i].free && blocks[i + 1].free) {
            blocks[i].size += blocks[i + 1].size;

            memmove(&blocks[i + 1], &blocks[i + 2],
                    (block_count - i - 2) * sizeof(Block));
            block_count--;
        } else {
            i++;
        }
    }
}

static void *my_malloc(size_t size) {
    if (size == 0 || size > REGION_SIZE)
        return NULL;

    size = align_size(size);
    size_t chosen = (size_t)-1;

    for (size_t i = 0; i < block_count; i++) {
        if (!blocks[i].free || blocks[i].size < size)
            continue;

        if (strategy == 0) {
            chosen = i;
            break;
        }

        if (chosen == (size_t)-1 ||
            blocks[i].size < blocks[chosen].size)
            chosen = i;
    }

    if (chosen == (size_t)-1)
        return NULL;

    if (blocks[chosen].size > size) {
        if (block_count >= MAX_BLOCKS)
            return NULL;

        memmove(&blocks[chosen + 2], &blocks[chosen + 1],
                (block_count - chosen - 1) * sizeof(Block));

        blocks[chosen + 1] = (Block){
            blocks[chosen].offset + size,
            blocks[chosen].size - size,
            1
        };

        blocks[chosen].size = size;
        block_count++;
    }

    blocks[chosen].free = 0;
    return region + blocks[chosen].offset;
}

static void my_free(void *ptr) {
    if (ptr == NULL || region == NULL)
        return;

    uintptr_t address = (uintptr_t)ptr;
    uintptr_t base = (uintptr_t)region;

    if (address < base || address >= base + REGION_SIZE)
        return;

    size_t offset = (size_t)(address - base);

    for (size_t i = 0; i < block_count; i++) {
        if (blocks[i].offset == offset && !blocks[i].free) {
            blocks[i].free = 1;
            merge_free();
            return;
        }
    }
}

static void *my_realloc(void *ptr, size_t new_size) {
    if (ptr == NULL)
        return my_malloc(new_size);

    if (new_size == 0) {
        my_free(ptr);
        return NULL;
    }

    uintptr_t address = (uintptr_t)ptr;
    uintptr_t base = (uintptr_t)region;

    if (address < base || address >= base + REGION_SIZE)
        return NULL;

    size_t offset = (size_t)(address - base);
    size_t index = block_count;

    for (size_t i = 0; i < block_count; i++) {
        if (blocks[i].offset == offset && !blocks[i].free) {
            index = i;
            break;
        }
    }

    if (index == block_count)
        return NULL;

    size_t old_size = blocks[index].size;
    size_t aligned = align_size(new_size);

    if (aligned <= old_size) {
        if (aligned < old_size && block_count < MAX_BLOCKS) {
            size_t remainder = old_size - aligned;

            memmove(&blocks[index + 2], &blocks[index + 1],
                    (block_count - index - 1) * sizeof(Block));

            blocks[index].size = aligned;
            blocks[index + 1] = (Block){
                offset + aligned, remainder, 1
            };

            block_count++;
            merge_free();
        }
        return ptr;
    }

    void *new_ptr = my_malloc(new_size);

    if (new_ptr == NULL)
        return NULL;

    memcpy(new_ptr, ptr, old_size);
    my_free(ptr);
    return new_ptr;
}

static void print_map(void) {
    printf("Memory blocks: ");

    for (size_t i = 0; i < block_count; i++) {
        printf("[%zu %s] ",
               blocks[i].size,
               blocks[i].free ? "FREE" : "USED");
    }

    printf("\n");
}

static void *allocate_memory(int custom, size_t size) {
    return custom ? my_malloc(size) : malloc(size);
}

static void release_memory(int custom, void *ptr) {
    if (custom)
        my_free(ptr);
    else
        free(ptr);
}

static void run_scenario(const char *name, int custom,
                         int selected_strategy, int scenario) {
    void *items[MAX_ITEMS] = {0};
    size_t count = 0;
    size_t successful = 0;

    if (custom) {
        strategy = selected_strategy;

        if (!init_allocator()) {
            printf("Cannot initialize custom allocator\n");
            return;
        }
    }

    clock_t begin = clock();

    if (scenario == 0) {
        /* Many small allocations */
        count = 1000;

        for (size_t i = 0; i < count; i++) {
            items[i] = allocate_memory(custom, 32);
            if (items[i] != NULL)
                successful++;
        }
    } else if (scenario == 1) {
        /* Alternating allocation and free */
        count = 512;

        for (size_t i = 0; i < count; i++) {
            items[i] = allocate_memory(custom, 128);
            if (items[i] != NULL)
                successful++;
        }

        for (size_t i = 0; i < count; i += 2) {
            release_memory(custom, items[i]);
            items[i] = NULL;
        }

        for (size_t i = 0; i < count; i += 2) {
            items[i] = allocate_memory(custom, 64);
            if (items[i] != NULL)
                successful++;
        }
    } else {
        /* Large allocations */
        count = 64;

        for (size_t i = 0; i < count; i++) {
            items[i] = allocate_memory(custom, 8192);
            if (items[i] != NULL)
                successful++;
        }
    }

    clock_t end = clock();

    for (size_t i = 0; i < count; i++)
        release_memory(custom, items[i]);

    double elapsed = 0.0;

    if (CLOCKS_PER_SEC != 0)
        elapsed = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("%-12s %-10s %-10s allocations=%zu time=%.6f sec\n",
           name,
           custom ? "Custom" : "glibc",
           custom ? (selected_strategy == 0 ? "FirstFit" : "BestFit") : "-",
           successful, elapsed);

    if (custom) {
        print_map();
        munmap(region, REGION_SIZE);
        region = NULL;
    }
}

int main(void) {
    printf("PR9 C: Custom mmap memory allocator\n");
    printf("====================================\n");

    const char *scenarios[] = {
        "Small blocks",
        "Alternating",
        "Large blocks"
    };

    for (int scenario = 0; scenario < 3; scenario++) {
        printf("\nScenario: %s\n", scenarios[scenario]);

        run_scenario(scenarios[scenario], 0, 0, scenario);
        run_scenario(scenarios[scenario], 1, 0, scenario);
        run_scenario(scenarios[scenario], 1, 1, scenario);
    }

    printf("\nRealloc test:\n");

    strategy = 0;

    if (init_allocator()) {
        char *ptr = my_malloc(100);

        if (ptr != NULL) {
            strcpy(ptr, "Memory allocator test");
            ptr = my_realloc(ptr, 200);

            if (ptr != NULL)
                printf("Realloc successful: %s\n", ptr);

            my_free(ptr);
        }

        print_map();
        munmap(region, REGION_SIZE);
        region = NULL;
    }

    struct rusage usage;

    if (getrusage(RUSAGE_SELF, &usage) == 0)
        printf("\nPeak RSS: %ld KB\n", usage.ru_maxrss);

    printf("\nAllocator tests completed.\n");
    return 0;
}

