#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

#define SIZE 4096

int main(void) {
    int fd = open("shared.dat", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    char *memory = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    strcpy(memory, "Hello from shared memory");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child process:\n");
        printf("Virtual address: %p\n", (void *)memory);
        printf("Shared data: %s\n", memory);
        strcpy(memory, "Message written by child");
        munmap(memory, SIZE);
        close(fd);
        return 0;
    }

    wait(NULL);
    printf("\nParent process:\n");
    printf("Virtual address: %p\n", (void *)memory);
    printf("Shared data: %s\n", memory);

    munmap(memory, SIZE);
    close(fd);
    unlink("shared.dat");
    return 0;
}
