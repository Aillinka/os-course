# Practical Work 11 — Page Replacement Algorithms

## Goal
Study page replacement algorithms and compare their performance.

## Variant A
Implemented FIFO, LRU and Optimal page replacement algorithms.
Compared page faults using 3 and 4 frames.

Reference string:
7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1

Expected results with 3 frames:
- FIFO: 15 page faults
- LRU: 12 page faults
- Optimal: 9 page faults

Optimal provides a theoretical lower bound but requires
knowledge of future page references.

## Variant B
Implemented Clock (Second Chance) algorithm.
Compared FIFO, LRU, Optimal and Clock with 1-8 frames.

Tested Belady's anomaly using:
1 2 3 4 1 2 5 1 2 3 4 5

FIFO:
- 3 frames: 9 page faults
- 4 frames: 10 page faults

LRU and Optimal do not demonstrate Belady's anomaly.

## Variant C
Implemented a C program that allocates 128 MB of memory.
Measured minor and major page faults using getrusage().
Compared first and repeated memory access.

Major page faults depend on actual system memory pressure
and may be zero in a normal environment.

## Graph
pr11_results.png compares page faults for different
numbers of frames.

## Conclusion
Page replacement algorithms have different performance.
LRU benefits from locality of reference, Optimal provides
a theoretical minimum, and FIFO may demonstrate Belady's anomaly.
