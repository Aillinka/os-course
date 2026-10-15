import csv
import statistics
from collections import defaultdict
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

sum_times = defaultdict(list)

with open("sum_results.csv", newline="") as f:
    for row in csv.DictReader(f):
        sum_times[int(row["threads"])].append(
            float(row["time_seconds"])
        )

threads = sorted(sum_times)
medians = [statistics.median(sum_times[n]) for n in threads]
baseline = medians[0]
speedups = [baseline / t for t in medians]

plt.figure(figsize=(8, 5))
plt.plot(threads, speedups, marker="o", label="Measured")
plt.plot(threads, threads, linestyle="--", label="Ideal")
plt.xlabel("Number of threads")
plt.ylabel("Speedup")
plt.title("Parallel Sum — 100 Million Elements")
plt.xticks(threads)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("speedup.png", dpi=160)
plt.close()

false_times = defaultdict(list)

with open("false_sharing_results.csv", newline="") as f:
    for row in csv.DictReader(
        f, fieldnames=["mode", "time_seconds"]
    ):
        false_times[row["mode"]].append(
            float(row["time_seconds"])
        )

modes = ["shared", "padded"]
values = [statistics.median(false_times[m]) for m in modes]

plt.figure(figsize=(7, 5))
plt.bar(modes, values)
plt.ylabel("Time (seconds)")
plt.title("False Sharing Comparison")
plt.tight_layout()
plt.savefig("false_sharing.png", dpi=160)
plt.close()

print("=== PARALLEL SUM ===")
for n, t, s in zip(threads, medians, speedups):
    print(f"{n} threads: {t:.6f}s, speedup={s:.2f}x")

print("\n=== FALSE SHARING ===")
for m, t in zip(modes, values):
    print(f"{m}: {t:.6f}s")

print("\nGraphs saved successfully!")
