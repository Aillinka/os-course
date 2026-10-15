from variant_a import fifo, lru, optimal

def clock(pages, capacity):
    frames = [None] * capacity
    bits = [0] * capacity
    pointer = 0
    faults = 0

    for page in pages:
        if page in frames:
            bits[frames.index(page)] = 1
        else:
            faults += 1

            while frames[pointer] is not None and bits[pointer] == 1:
                bits[pointer] = 0
                pointer = (pointer + 1) % capacity

            frames[pointer] = page
            bits[pointer] = 1
            pointer = (pointer + 1) % capacity

    return faults


REFERENCE = [7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
             0, 3, 2, 1, 2, 0, 1, 7, 0, 1]

BELADY = [1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5]

algorithms = {
    "FIFO": fifo,
    "LRU": lru,
    "Optimal": optimal,
    "Clock": clock
}

print("COMPARISON: FRAMES 1-8")

for name, algorithm in algorithms.items():
    print("\n", name)
    for frames in range(1, 9):
        print(frames, "frames:", algorithm(REFERENCE, frames), "faults")


print("\nBELADY'S ANOMALY")

for frames in [3, 4]:
    print("FIFO", frames, "frames:", fifo(BELADY, frames))

    print("LRU", frames, "frames:", lru(BELADY, frames))

    print("Optimal", frames, "frames:", optimal(BELADY, frames))
