from collections import OrderedDict
import matplotlib.pyplot as plt

PAGE_SIZE = 1024

PAGE_TABLE = {
    0: 5, 1: 2, 2: 7, 3: 0,
    4: None, 5: 4, 6: 1, 7: 6,
    8: None, 9: 3
}

REFERENCES = [
    1, 3, 1, 2, 3, 1, 5, 2, 1, 3,
    6, 7, 1, 2, 5, 3, 7, 6, 1, 2
]


def simulate_tlb(capacity):
    tlb = OrderedDict()
    hits = 0
    misses = 0

    for page in REFERENCES:
        if page in tlb:
            hits += 1
            tlb.move_to_end(page)
        else:
            misses += 1

            frame = PAGE_TABLE.get(page)

            if frame is not None:
                tlb[page] = frame

                if len(tlb) > capacity:
                    tlb.popitem(last=False)

    total = hits + misses
    hit_ratio = hits / total if total else 0

    return hits, misses, hit_ratio


def calculate_eat(hit_ratio):
    tlb_time = 10
    memory_time = 100

    return hit_ratio * (tlb_time + memory_time) + (
        1 - hit_ratio
    ) * (tlb_time + 2 * memory_time)


def two_level_translation(address):
    page = address // PAGE_SIZE
    offset = address % PAGE_SIZE

    first_level = page // 4
    second_level = page % 4

    frame = PAGE_TABLE.get(page)

    print(f"Address: {address}")
    print(f"First-level index: {first_level}")
    print(f"Second-level index: {second_level}")
    print(f"Offset: {offset}")

    if frame is None:
        print("Page is not present")
    else:
        print(f"Frame: {frame}")
        print(f"Physical address: {frame * PAGE_SIZE + offset}")


def main():
    print("PRACTICAL WORK 10 - VARIANT B")
    print("=" * 50)

    print("\nTLB LRU simulation:")
    print("Capacity | Hits | Misses | Hit ratio | EAT (ns)")

    sizes = [1, 2, 3, 4, 5, 6, 8]
    ratios = []

    for size in sizes:
        hits, misses, ratio = simulate_tlb(size)
        eat = calculate_eat(ratio)
        ratios.append(ratio)

        print(
            f"{size:8} | {hits:4} | {misses:6} | "
            f"{ratio:9.3f} | {eat:8.2f}"
        )

    plt.figure(figsize=(8, 5))
    plt.plot(sizes, ratios, marker="o")
    plt.xlabel("TLB capacity (entries)")
    plt.ylabel("TLB hit ratio")
    plt.title("TLB size vs hit ratio")
    plt.grid(True)
    plt.savefig("tlb_results.png")
    plt.close()

    print("\nGraph saved as tlb_results.png")

    print("\nTwo-level page table example:")
    two_level_translation(1200)

    print("\nEffective Access Time examples:")
    for ratio in [0.0, 0.5, 0.8, 0.9, 1.0]:
        print(
            f"Hit ratio={ratio:.1f}: "
            f"EAT={calculate_eat(ratio):.2f} ns"
        )


if __name__ == "__main__":
    main()
