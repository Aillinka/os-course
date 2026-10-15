# Practical Work 8 — Deadlocks

## Objective

To investigate deadlocks, prevention strategies,
deadlock detection tools, and the Banker's algorithm.

## Environment

Ubuntu Linux, C11, POSIX threads, Python 3.

## Dining Philosophers

Four approaches are implemented:

1. Naive — demonstrates circular waiting.
2. Ordered — acquires mutexes in a fixed order.
3. Trylock — releases acquired resources if the
   second resource is unavailable.
4. Waiter — limits the number of philosophers
   competing for forks to N-1.

## Compilation

    gcc -std=c11 -O1 -g -Wall -Wextra -pthread -o philosophers philosophers.c

## Execution

    ./philosophers naive
    ./philosophers ordered
    ./philosophers trylock
    ./philosophers waiter

The working variants run for approximately
10 seconds and report:

- Meals completed by each philosopher
- Maximum observed waiting time

The naive variant demonstrates a deadlock
and is terminated without waiting indefinitely.

## Helgrind

    valgrind --tool=helgrind ./philosophers ordered

Helgrind can detect inconsistent lock ordering
and potential data races.

It cannot guarantee detection of every deadlock
or prove that a program is deadlock-free.

## Linux Kernel

Linux lockdep checks locking dependencies
inside the kernel when enabled.

The hung_task detector can report kernel tasks
blocked for an unusually long time.

These mechanisms do not automatically resolve
deadlocks between ordinary userspace pthread
mutexes.

## Banker's Algorithm

Run:

    python3 banker.py bank1
    python3 banker.py bank2

Bank1 is a safe state.

Bank2 is an unsafe state.

The safety algorithm calculates:

    Need = Maximum - Allocation

It finds a process whose remaining needs
can be satisfied by the available resources.

After the process finishes, its allocated
resources are returned to Work.

## Conclusion

Deadlocks can occur when processes hold
resources while waiting for other resources.

A global resource acquisition order
prevents circular waiting.

Trylock with rollback avoids indefinite
blocking, although poor retry strategies
may lead to livelock or starvation.

The waiter solution prevents all philosophers
from holding one fork simultaneously.

The Banker's algorithm checks whether the
system can complete all processes safely.
