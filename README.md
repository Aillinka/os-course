# Operating Systems — Practical Works

**Level:** C
**Environment:** Ubuntu Linux

This repository contains practical assignments
for the Operating Systems course.

## Practical Works

- PR1 — Linux Installation, SSH and systemd
- PR2 — Bash and System Calls
- PR3 — Linux Processes
- PR4 — Threads
- PR5 — CPU Scheduling
- PR6 — Synchronization
- PR7 — Interprocess Communication
- PR8 — Deadlocks

Each practical work is organized in a separate
directory and includes source code, results,
and documentation.

## Additional Experiment — Locality and Random Access

The locality experiment generates memory references where
most accesses are concentrated in a small group of pages.

The random experiment generates references across a wider
range of pages.

Both sequences are tested with FIFO, LRU, Optimal and Clock
using five frames.

The experiment demonstrates how locality of reference
can influence page replacement performance.

## Conclusion

LRU can benefit from locality because recently used pages
are likely to be accessed again. Optimal gives a theoretical
minimum number of page faults when future references are known.
FIFO may demonstrate Belady's anomaly.
