# Practical Work 7 — Interprocess Communication

## Objective
Implement and compare four IPC mechanisms in Linux using C.

## IPC Mechanisms
1. Pipe — communication using read() and write().
2. POSIX Message Queue — mq_open(), mq_send(), mq_receive().
3. POSIX Shared Memory — shm_open(), mmap(), semaphores.
4. Unix Domain Socket — socketpair().

## Compilation
gcc -O2 -Wall -Wextra -o ipc_benchmark ipc_benchmark.c
gcc -O2 -Wall -Wextra -pthread -o ipc_extra ipc_extra.c -lrt

## Quick Test
./ipc_benchmark 4096 4
./ipc_extra 4096 4

## Benchmark
./run_tests.sh 4
./run_tests.sh 1024

Message sizes:
- 64 bytes
- 4096 bytes
- 1048576 bytes

The full benchmark transfers 1 GiB per test.

## System Call Analysis
strace -f -c ./ipc_benchmark 4096 4
strace -f -c ./ipc_extra 4096 4

## Performance Analysis
perf stat ./ipc_benchmark 4096 4
perf stat ./ipc_extra 4096 4

## Explanation
Pipe and Unix sockets transfer data through the kernel.
They require system calls for sending and receiving data.

POSIX message queues preserve message boundaries but have
a limited maximum message size.

Shared memory allows processes to access the same memory
region without copying each message through the kernel.
However, synchronization is necessary to avoid race conditions.

## SIGTERM
The program installs a SIGTERM handler and attempts to stop
the child process and release IPC resources.

The correctness of cleanup must be verified using:
ls /dev/shm
ls /dev/mqueue

## Control Questions

### Why is shared memory fast but dangerous?
Shared memory allows processes to access the same memory
without repeated data copying. However, simultaneous access
without synchronization can cause race conditions.

### What happens when writing to a pipe with no readers?
The operating system generates SIGPIPE. If SIGPIPE is ignored
or handled, write() fails with EPIPE.

## Conclusion
The practical work demonstrates different methods of
interprocess communication and compares their performance.
Results depend on message size, synchronization overhead,
system calls, and kernel involvement.
