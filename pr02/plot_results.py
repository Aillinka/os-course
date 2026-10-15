import csv
import statistics
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

results = defaultdict(list)

with open("results.csv", newline="") as file:
    reader = csv.DictReader(file)

    for row in reader:
        method = row["method"]
        size = int(row["buffer_size"])
        elapsed = float(row["time_seconds"])

        results[(method, size)].append(elapsed)

sizes = [1, 64, 4096, 65536]

plt.figure(figsize=(9, 5))

for method in ["rw", "stdio"]:
    medians = [
        statistics.median(results[(method, size)])
        for size in sizes
    ]

    print(f"{method}:")
    for size, elapsed in zip(sizes, medians):
        print(f"  {size} bytes: {elapsed:.6f} seconds")

    plt.plot(sizes, medians, marker="o", label=method)

if ("sendfile", 65536) in results:
    sendfile_time = statistics.median(results[("sendfile", 65536)])
    print(f"sendfile median: {sendfile_time:.6f} seconds")

plt.xscale("log", base=2)
plt.xlabel("Buffer size (bytes)")
plt.ylabel("Copy time (seconds)")
plt.title("File Copy Performance — 100 MiB")
plt.grid(True)
plt.legend()
plt.tight_layout()

plt.savefig("benchmark.png", dpi=160)
print("Graph saved: benchmark.png")
