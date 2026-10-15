# Practical Work 4 — Threads in Linux

**Course:** Operating Systems
**Level:** C
**Environment:** Ubuntu Linux, ARM64

## Objective

To investigate multithreading using POSIX threads,
measure parallel speedup, compare thread and process
creation costs, and demonstrate false sharing.

## Task 1 — Parallel Sum

A C program calculates the sum of an array containing
100 million unsigned integers.

The experiment uses:
- 1 thread
- 2 threads
- 4 threads
- 8 threads
- 16 threads

Each configuration is repeated three times.

The array is initialized before timing begins.
The result is checked for correctness.

Results: sum_results.csv
Graph: speedup.png

Speedup is calculated as:

Speedup = T(1) / T(N)

## Task 2 — Creation Cost

The program compares:
- pthread_create() followed by pthread_join()
- fork() followed by waitpid()

Each operation is repeated 10,000 times.

The experiment measures creation and completion
overhead, not just the creation function itself.

Results:
- creation_results.csv
- creation_repeats.txt

## Task 3 — False Sharing

Two threads update separate counters.

In the first experiment, counters are adjacent
in memory.

In the second experiment, counters are separated
using 64-byte alignment and padding.

Five measurements are collected for each case.

Results: false_sharing_results.csv
Graph: false_sharing.png

## Compilation

    gcc -O2 -Wall -Wextra -pthread -o parallel_sum parallel_sum.c
    gcc -O2 -Wall -Wextra -pthread -o creation_cost creation_cost.c
    gcc -std=c11 -O2 -Wall -Wextra -pthread -o false_sharing false_sharing.c

## Execution

    ./run_sum.sh
    ./creation_cost
    ./false_sharing
    python3 plot_results.py

## Discussion

Parallel speedup depends on available CPU cores,
memory bandwidth, thread overhead, and cache behavior.

False sharing can reduce performance when independent
variables share a cache line.

Threads share an address space and are useful for
parallel tasks requiring shared data.

Processes have separate virtual address spaces and
are useful when isolation is important.

## Conclusion

This practical work demonstrates parallel execution
with POSIX threads and investigates its performance.

Adding more threads does not guarantee unlimited
speedup. Performance depends on hardware resources,
memory access patterns, and synchronization overhead.

The exact experimental results are provided in the
CSV files and graphs.
