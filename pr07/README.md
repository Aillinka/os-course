# Practical Work 7 — Interprocess Communication

**Course:** Operating Systems
**Level:** C
**Environment:** Ubuntu Linux

## Objective

To compare four Linux interprocess communication
mechanisms using two processes.

## Implemented Methods

1. Anonymous pipe
2. POSIX message queue
3. POSIX shared memory with semaphores
4. Unix domain socket

## Benchmark Configuration

Total transfer size: 1024 MiB (1 GiB).

Logical message sizes:
- 64 bytes
- 4096 bytes
- 1048576 bytes

Large logical messages are divided into
smaller chunks for POSIX message queues.

## Compilation

    gcc -std=c11 -O2 -Wall -Wextra -pthread -o ipc_benchmark ipc_benchmark.c

## Quick Test

    ./run_benchmark.sh 4

## Full Benchmark

    ./run_benchmark.sh 1024

## Results

Measured results are stored in results.csv.

The program reports:
- Execution time
- Throughput in MiB/s
- Average time per logical message
- Data verification result

## Correctness

The receiving process checks the transmitted
data against a known byte pattern.

Shared memory access is synchronized using
process-shared POSIX semaphores.

## Resource Cleanup

The program uses shm_unlink and mq_unlink
to remove named IPC resources.

SIGTERM and SIGINT handlers request
graceful termination.

## System Call Analysis

System call statistics are collected
using strace -f -c.

perf stat is used when supported by
the Linux environment.

## Conclusion

Pipes and Unix sockets transfer data through
kernel-managed communication mechanisms.

POSIX message queues preserve message
boundaries but have message size limits.

Shared memory allows processes to access
the same memory region without copying
every message through a pipe or socket.

However, shared memory requires explicit
synchronization to prevent incorrect access.

Actual performance depends on message size,
kernel configuration, and system load.
