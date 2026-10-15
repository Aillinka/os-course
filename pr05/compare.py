import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from sched import load_processes, simulate, metrics

rows = []

for dataset in ["set1.txt", "set2.txt", "set3.txt"]:
    processes = load_processes(dataset)

    for algorithm in ["srtf", "mlfq"]:
        timeline = simulate(processes, algorithm, boost=30)
        result = metrics(processes, timeline)

        rows.append({
            "dataset": dataset,
            "algorithm": algorithm,
            **result
        })

        print(dataset, algorithm, result)

with open("comparison.csv", "w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=rows[0].keys())
    writer.writeheader()
    writer.writerows(rows)

labels = ["set1", "set2", "set3"]
srtf = [r["waiting"] for r in rows if r["algorithm"] == "srtf"]
mlfq = [r["waiting"] for r in rows if r["algorithm"] == "mlfq"]

x = range(3)
plt.figure(figsize=(8, 5))
plt.bar([i - 0.2 for i in x], srtf, width=0.4, label="SRTF")
plt.bar([i + 0.2 for i in x], mlfq, width=0.4, label="MLFQ")
plt.xticks(list(x), labels)
plt.ylabel("Average waiting time")
plt.title("MLFQ vs SRTF")
plt.legend()
plt.tight_layout()
plt.savefig("comparison.png", dpi=160)

print("Saved comparison.csv and comparison.png")
