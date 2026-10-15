#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(void) {
    const char message[] = "Hello from Linux!\n";
    size_t length = strlen(message);

    ssize_t normal = write(
        STDOUT_FILENO,
        message,
        length
    );

    ssize_t direct = syscall(
        SYS_write,
        STDOUT_FILENO,
        message,
        length
    );

    if (normal < 0 || direct < 0) {
        perror("write");
        return 1;
    }

    printf("Normal write: %zd bytes\n", normal);
    printf("Direct syscall: %zd bytes\n", direct);

    return normal == direct ? 0 : 1;
}
