# Practical Work 2 — Bash and System Calls

**Course:** Operating Systems
**Level:** C
**Environment:** Ubuntu Linux (ARM64)

## Objective

The objective of this practical work is to investigate
Linux system calls and compare file copying performance
using different I/O methods and buffer sizes.

## Implemented Methods

Three file copying methods were implemented in C:

1. read() and write()
2. fread() and fwrite()
3. sendfile() (additional experiment)

## Experimental Setup

The test file size was 100 MiB.

Four buffer sizes were tested:

- 1 byte
- 64 bytes
- 4096 bytes
- 65536 bytes

Each configuration was repeated three times.

The output files were verified using cmp.

## Compilation

    gcc -O2 -Wall -Wextra -o copy_benchmark copy_benchmark.c
    gcc -Wall -Wextra -o syscall_demo syscall_demo.c

## Running the Experiments

First, generate the test file:

    dd if=/dev/zero of=test_100mb.bin bs=1M count=100

Then compile and run:

    ./benchmark.sh
    python3 plot_results.py

## Results

The raw measurements are stored in results.csv.

The graph is available in benchmark.png.

The graph compares file copying time for different
buffer sizes and I/O methods.

## System Call Analysis

System call statistics were collected using strace.

Results:

- strace_rw.txt
- strace_stdio.txt
- strace_sendfile.txt

Small buffers generally require more I/O operations,
which can increase system call overhead.

Larger buffers usually reduce the number of calls.

The stdio functions use library-level buffering,
which can change the number of kernel interactions.

## Direct System Call Demonstration

The syscall_demo.c program compares:

- write()
- syscall(SYS_write, ...)

Both methods produce the same output.

## Additional Experiment

The sendfile() system call was tested as an
additional file copying method.

It can transfer file data within the kernel,
reducing some user-space copying overhead.

## Conclusion

The experiments demonstrate that file copying
performance depends on buffer size, I/O method,
and system call overhead.

The exact results also depend on caching,
storage performance, and virtualization.

The practical work was performed on Ubuntu Linux.
