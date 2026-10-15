import matplotlib.pyplot as plt
from variant_a import REFERENCE, fifo, lru, optimal
from variant_b import clock

frames = range(1, 9)

algorithms = {
    "FIFO": fifo,
    "LRU": lru,
    "Optimal": optimal,
    "Clock": clock
}

for name, algorithm in algorithms.items():
    results = [algorithm(REFERENCE, n) for n in frames]
    plt.plot(list(frames), results, marker="o", label=name)

plt.xlabel("Number of frames")
plt.ylabel("Page faults")
plt.title("Page Replacement Algorithms")
plt.xticks(list(frames))
plt.grid(True)
plt.legend()
plt.savefig("pr11_results.png")
print("Graph saved: pr11_results.png")
