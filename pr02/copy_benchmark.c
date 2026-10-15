#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int copy_rw(const char *src, const char *dst, size_t size) {
    int in = open(src, O_RDONLY);
    if (in < 0) {
        perror("open source");
        return 1;
    }

    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) {
        perror("open destination");
        close(in);
        return 1;
    }

    char *buffer = malloc(size);
    if (!buffer) {
        perror("malloc");
        close(in);
        close(out);
        return 1;
    }

    int error = 0;
    ssize_t n;

    while (1) {
        n = read(in, buffer, size);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
            break;

        ssize_t written = 0;

        while (written < n) {
            ssize_t w = write(out, buffer + written, n - written);

            if (w < 0 && errno == EINTR)
                continue;

            if (w <= 0) {
                perror("write");
                error = 1;
                break;
            }

            written += w;
        }

        if (error)
            break;
    }

    if (n < 0) {
        perror("read");
        error = 1;
    }

    free(buffer);
    if (close(in) != 0) error = 1;
    if (close(out) != 0) error = 1;

    return error;
}

static int copy_stdio(const char *src, const char *dst, size_t size) {
    FILE *in = fopen(src, "rb");
    if (!in) {
        perror("fopen source");
        return 1;
    }

    FILE *out = fopen(dst, "wb");
    if (!out) {
        perror("fopen destination");
        fclose(in);
        return 1;
    }

    char *buffer = malloc(size);
    if (!buffer) {
        perror("malloc");
        fclose(in);
        fclose(out);
        return 1;
    }

    int error = 0;
    size_t n;

    while ((n = fread(buffer, 1, size, in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            perror("fwrite");
            error = 1;
            break;
        }
    }

    if (ferror(in)) {
        perror("fread");
        error = 1;
    }

    free(buffer);
    if (fclose(in) != 0) error = 1;
    if (fclose(out) != 0) error = 1;

    return error;
}

static int copy_sendfile(const char *src, const char *dst) {
    int in = open(src, O_RDONLY);
    if (in < 0) {
        perror("open source");
        return 1;
    }

    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) {
        perror("open destination");
        close(in);
        return 1;
    }

    struct stat st;

    if (fstat(in, &st) != 0) {
        perror("fstat");
        close(in);
        close(out);
        return 1;
    }

    off_t offset = 0;
    int error = 0;

    while (offset < st.st_size) {
        size_t remaining = (size_t)(st.st_size - offset);
        size_t chunk = remaining > 1048576 ? 1048576 : remaining;

        ssize_t n = sendfile(out, in, &offset, chunk);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0) {
            perror("sendfile");
            error = 1;
            break;
        }
    }

    if (close(in) != 0) error = 1;
    if (close(out) != 0) error = 1;

    return error;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr,
            "Usage: %s MODE BUFFER_SIZE SOURCE DESTINATION\n",
            argv[0]);
        return 1;
    }

    char *end;
    errno = 0;

    unsigned long long parsed = strtoull(argv[2], &end, 10);

    if (argv[2][0] == '-' || errno || end == argv[2] ||
        *end != '\0' || parsed == 0 || parsed > 1048576) {
        fprintf(stderr, "Invalid buffer size\n");
        return 1;
    }

    if (strcmp(argv[3], argv[4]) == 0) {
        fprintf(stderr, "Source and destination must differ\n");
        return 1;
    }

    size_t buffer_size = (size_t)parsed;
    double start = now_seconds();
    int result;

    if (strcmp(argv[1], "rw") == 0) {
        result = copy_rw(argv[3], argv[4], buffer_size);
    } else if (strcmp(argv[1], "stdio") == 0) {
        result = copy_stdio(argv[3], argv[4], buffer_size);
    } else if (strcmp(argv[1], "sendfile") == 0) {
        result = copy_sendfile(argv[3], argv[4]);
    } else {
        fprintf(stderr, "Unknown mode\n");
        return 1;
    }

    double elapsed = now_seconds() - start;

    if (result == 0) {
        printf("%s,%zu,%.6f\n", argv[1], buffer_size, elapsed);
    }

    return result;
}
