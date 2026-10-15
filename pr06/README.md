
## Spinlock and Helgrind Analysis

The spinlock was implemented using C11 atomic
operations with acquire/release memory ordering.

Helgrind reported 179 errors from 5 contexts
during the spinlock benchmark.

The reported accesses involved:
- lock_spin()
- unlock_spin()
- the shared counter

Helgrind does not necessarily recognize custom
atomic spinlocks as synchronization primitives.

The benchmark verified the final counter value
for every tested configuration.

However, the Helgrind warnings cannot be
classified as false positives solely from
this result. Further verification would be
needed to establish that no races remain.

The pthread-based reader-writer implementations
were separately checked with Helgrind.

Both reader and fair modes reported 0 errors.

## Final Race Condition Test

Two programs were tested with Helgrind.

### race_demo.c
- Expected counter: 400000
- Actual counter: 400000
- Helgrind: 6 errors from 2 contexts
- Result: data race detected

### race_fixed.c
- Expected counter: 400000
- Actual counter: 400000
- Helgrind: 0 errors
- Result: race condition fixed using pthread_mutex_t

### Reader-Writer Verification
- Reader mode: 0 Helgrind errors
- Fair mode: 0 Helgrind errors
- Final data: 2000
- Invariant violations: 0

The experiment shows that a correct final counter
does not guarantee the absence of a data race.
