# Practical Work 8 — Deadlocks

## Objective
Study deadlocks, prevention methods and the Banker's Algorithm.

## Dining Philosophers

The program is implemented in C using pthreads.

Four modes are included:

1. naive — demonstrates deadlock.
2. ordered — locks forks in ascending order.
3. trylock — releases acquired resources if the second lock is unavailable.
4. waiter — uses a semaphore with N-1 permits.

## Compilation

gcc -O2 -Wall -Wextra -pthread -o philosophers philosophers.c

gcc -O2 -Wall -Wextra -pthread \
-o philosophers_deadlock philosophers_deadlock.c

## Run

./philosophers ordered
./philosophers trylock
./philosophers waiter

## Deadlock Demonstration

timeout 5s ./philosophers_deadlock naive
echo $?

Exit code 124 means the process exceeded the time limit.

In the naive implementation, every philosopher first
acquires one fork and then waits for the next fork.
This creates circular waiting.

## Results

The programs record:
- Number of meals per philosopher.
- Maximum waiting time in milliseconds.
- Measurements over approximately 10 seconds.

Results are stored in:
- ordered_results.txt
- trylock_results.txt
- waiter_results.txt

The naive deadlock demonstration does not complete
a normal 10-second measurement.

## Banker's Algorithm

Run:

python3 banker.py bank1.txt
python3 banker.py bank2.txt

Expected results:

bank1.txt — SAFE
bank2.txt — UNSAFE

The algorithm calculates Need = Max - Allocation.

When a process can finish, its allocated resources
are returned to Work.

## Helgrind

Run:

valgrind --tool=helgrind ./philosophers ordered
valgrind --tool=helgrind ./philosophers trylock
valgrind --tool=helgrind ./philosophers waiter

Helgrind detects potential data races and lock-order
violations. It does not guarantee detection of every
deadlock or livelock.

In this experiment, the ordered mode produced
0 reported errors.

The trylock and waiter modes produced Helgrind
warnings that require interpretation.

## Linux Deadlock Detection

lockdep is a Linux kernel debugging mechanism that
tracks lock dependencies and detects potentially
dangerous lock acquisition orders.

hung_task detects tasks that remain blocked in
certain uninterruptible kernel states for too long.

The Linux kernel does not generally detect deadlocks
between arbitrary userspace pthread mutexes because
it does not maintain a complete userspace resource
allocation graph.

## Control Questions

### Why does ordered locking prevent deadlock?

All threads acquire resources in the same global order.
This prevents circular waiting, one of the four
necessary conditions for deadlock.

### Why is the Banker's Algorithm rarely used in general OS?

The algorithm requires knowledge of maximum future
resource demands. General-purpose operating systems
usually cannot predict these demands accurately.

## Conclusion

Deadlocks can occur when threads acquire shared
resources in conflicting orders.

Ordered locking prevents circular waiting.
Trylock with rollback avoids holding one fork
indefinitely while waiting for another.
The waiter limits the number of competing philosophers.

The Banker's Algorithm identifies safe and unsafe
resource allocation states.
