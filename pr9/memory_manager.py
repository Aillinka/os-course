
import random
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

MEMORY_SIZE = 1024


class Block:
    def __init__(self, start, size, pid=None):
        self.start = start
        self.size = size
        self.pid = pid

    @property
    def free(self):
        return self.pid is None


class MemoryManager:
    def __init__(self, strategy="first"):
        self.strategy = strategy
        self.blocks = [Block(0, MEMORY_SIZE)]
        self.failures = 0
        self.fragmentation = []

    def coalesce(self):
        i = 0
        while i < len(self.blocks) - 1:
            a = self.blocks[i]
            b = self.blocks[i + 1]

            if a.free and b.free:
                a.size += b.size
                self.blocks.pop(i + 1)
            else:
                i += 1

    def allocate(self, pid, size):
        candidates = [
            (i, block)
            for i, block in enumerate(self.blocks)
            if block.free and block.size >= size
        ]

        if not candidates:
            self.failures += 1
            return False

        if self.strategy == "first":
            index, block = candidates[0]
        elif self.strategy == "best":
            index, block = min(candidates, key=lambda x: x[1].size)
        else:
            index, block = max(candidates, key=lambda x: x[1].size)

        new_blocks = [Block(block.start, size, pid)]

        if block.size > size:
            new_blocks.append(
                Block(block.start + size, block.size - size)
            )

        self.blocks[index:index + 1] = new_blocks
        self.measure()
        return True

    def release(self, pid):
        found = False

        for block in self.blocks:
            if block.pid == pid:
                block.pid = None
                found = True

        if found:
            self.coalesce()
            self.measure()

        return found

    def measure(self):
        free_blocks = [
            block.size for block in self.blocks if block.free
        ]

        total_free = sum(free_blocks)
        largest = max(free_blocks, default=0)

        if total_free == 0:
            value = 0
        else:
            value = 1 - largest / total_free

        self.fragmentation.append(value)

    def show_memory(self):
        result = []

        for block in self.blocks:
            name = block.pid if block.pid else "FREE"
            result.append(
                f"[{block.start}:{block.start + block.size - 1} "
                f"{name} size={block.size}]"
            )

        return " | ".join(result)


def ops1():
    return [
        ("alloc", "A", 120),
        ("alloc", "B", 220),
        ("alloc", "C", 180),
        ("alloc", "D", 140),
        ("free", "B", 0),
        ("alloc", "E", 160),
        ("free", "D", 0),
        ("alloc", "F", 100),
        ("free", "A", 0),
        ("alloc", "G", 200),
        ("free", "C", 0),
        ("free", "E", 0)
    ]


def make_ops2():
    random.seed(209)
    operations = []
    active = []

    for i in range(200):
        if active and random.random() < 0.42:
            pid = random.choice(active)
            active.remove(pid)
            operations.append(("free", pid, 0))
        else:
            pid = f"P{i}"
            size = random.choice(
                [16, 24, 32, 48, 64, 80, 96, 128, 160, 192, 256]
            )
            active.append(pid)
            operations.append(("alloc", pid, size))

    return operations


def run_fit(operations, strategy, show=False):
    manager = MemoryManager(strategy)

    for operation, pid, size in operations:
        if operation == "alloc":
            success = manager.allocate(pid, size)
            status = "OK" if success else "FAILED"
        else:
            success = manager.release(pid)
            status = "released" if success else "unknown PID"

        if show:
            print(
                f"{operation.upper():5} {pid:5} "
                f"{status:8} {manager.show_memory()}"
            )

    free = [
        block.size for block in manager.blocks if block.free
    ]

    return manager, sum(free), max(free, default=0)


class BuddySystem:
    def __init__(self):
        self.free = {1024: [0]}
        self.allocated = {}
        self.failures = 0
        self.internal_waste = 0
        self.events = []

    def allocate(self, pid, requested):
        size = 1

        while size < requested:
            size *= 2

        if size > 1024:
            self.failures += 1
            return False

        candidate = size

        while candidate <= 1024 and not self.free.get(candidate):
            candidate *= 2

        if candidate > 1024:
            self.failures += 1
            return False

        start = self.free[candidate].pop(0)

        while candidate > size:
            candidate //= 2
            self.free.setdefault(candidate, []).append(start + candidate)
            self.events.append(
                f"Split: start={start}, buddy={start + candidate}, size={candidate}"
            )

        self.allocated[pid] = (start, size, requested)
        self.internal_waste += size - requested

        return True

    def release(self, pid):
        if pid not in self.allocated:
            return False

        start, size, requested = self.allocated.pop(pid)
        self.internal_waste -= size - requested

        while size < 1024:
            buddy = start ^ size

            if buddy not in self.free.get(size, []):
                break

            self.free[size].remove(buddy)
            start = min(start, buddy)
            size *= 2

            self.events.append(
                f"Merge: start={start}, size={size}"
            )

        self.free.setdefault(size, []).append(start)
        return True


def main():
    print("========== VARIANT A ==========")
    print("\nFirst Fit — ops1 memory map:")

    run_fit(ops1(), "first", True)

    print("\nFinal ops1 comparison:")

    for strategy in ["first", "best", "worst"]:
        manager, total_free, largest = run_fit(ops1(), strategy)

        external = (
            0 if total_free == 0
            else 1 - largest / total_free
        )

        print(
            f"{strategy.upper():6} "
            f"free={total_free:4} "
            f"largest={largest:4} "
            f"external_fragmentation={external:.3f} "
            f"failures={manager.failures}"
        )

    print("\n========== VARIANT B ==========")

    operations = make_ops2()
    results = {}

    for strategy in ["first", "best", "worst"]:
        manager, _, _ = run_fit(operations, strategy)

        average = (
            sum(manager.fragmentation) /
            max(1, len(manager.fragmentation))
        )

        results[strategy] = (
            manager.failures,
            average,
            0
        )

    buddy = BuddySystem()

    for operation, pid, size in operations:
        if operation == "alloc":
            buddy.allocate(pid, size)
        else:
            buddy.release(pid)

    results["buddy"] = (
        buddy.failures,
        0,
        buddy.internal_waste
    )

    print("\nops2 comparison:")
    print("Strategy  Failures  Avg_external  Buddy_internal_waste")

    for name, values in results.items():
        print(
            f"{name:8} "
            f"{values[0]:8} "
            f"{values[1]:12.3f} "
            f"{values[2]:10}"
        )

    print("\nBuddy split/merge events:")
    for event in buddy.events[:20]:
        print(event)

    names = list(results.keys())
    failures = [results[name][0] for name in names]

    plt.figure(figsize=(8, 4))
    plt.bar(names, failures)
    plt.xlabel("Memory allocation strategy")
    plt.ylabel("Failed allocations")
    plt.title("PR9: Allocation failures on ops2")
    plt.tight_layout()
    plt.savefig("pr9_results.png")

    print("\nGraph saved as pr9_results.png")


if __name__ == "__main__":
    main()

