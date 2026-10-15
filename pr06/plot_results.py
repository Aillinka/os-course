import csv
import statistics
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

results = defaultdict(list)

with open("lock_repeats.csv", newline="") as f:
    for row in csv.DictReader(f):
        key = (
            row["lock"],
            int(row["threads"]),
            row["section"]
        )
        results[key].append(float(row["seconds"]))

for section in ["short", "long"]:
    plt.figure(figsize=(8, 5))

    for lock in ["mutex", "spinlock"]:
        threads = [2, 4, 16]
        times = [
            statistics.median(results[(lock, n, section)])
            for n in threads
        ]

        plt.plot(threads, times, marker="o", label=lock)

        print(section, lock, times)

    plt.xlabel("Number of threads")
    plt.ylabel("Time (seconds)")
    plt.title(f"Mutex vs Spinlock — {section} section")
    plt.xticks([2, 4, 16])
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"locks_{section}.png", dpi=160)
    plt.close()

print("Graphs saved!")
