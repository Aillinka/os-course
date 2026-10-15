import random
from page_replacement import fifo, lru, optimal, clock

random.seed(42)

locality = []
for _ in range(100):
    locality.extend(random.choices(range(5), k=8))
    locality.extend(random.choices(range(5, 20), k=2))

random_refs = [random.randint(0, 19) for _ in range(1000)]

experiments = {
    "Locality sequence": locality,
    "Random sequence": random_refs
}

algorithms = {
    "FIFO": fifo,
    "LRU": lru,
    "Optimal": optimal,
    "Clock": clock
}

for sequence_name, sequence in experiments.items():
    print("\n", sequence_name)

    for name, algorithm in algorithms.items():
        faults = algorithm(sequence, 5)
        print(f"{name}: {faults} page faults")
