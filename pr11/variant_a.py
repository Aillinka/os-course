REFERENCE = [7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
             0, 3, 2, 1, 2, 0, 1, 7, 0, 1]


def fifo(pages, capacity):
    frames = []
    pointer = 0
    faults = 0

    print("\nFIFO:")
    for page in pages:
        if page not in frames:
            faults += 1
            if len(frames) < capacity:
                frames.append(page)
            else:
                frames[pointer] = page
                pointer = (pointer + 1) % capacity

        print(page, "->", frames, "Faults:", faults)

    return faults


def lru(pages, capacity):
    frames = []
    recent = []
    faults = 0

    print("\nLRU:")
    for page in pages:
        if page in frames:
            recent.remove(page)
            recent.append(page)
        else:
            faults += 1
            if len(frames) == capacity:
                old = recent.pop(0)
                frames.remove(old)

            frames.append(page)
            recent.append(page)

        print(page, "->", frames, "Faults:", faults)

    return faults


def optimal(pages, capacity):
    frames = []
    faults = 0

    print("\nOPTIMAL:")
    for i, page in enumerate(pages):
        if page not in frames:
            faults += 1

            if len(frames) < capacity:
                frames.append(page)
            else:
                future = pages[i + 1:]
                distances = []

                for item in frames:
                    if item in future:
                        distances.append(future.index(item))
                    else:
                        distances.append(float("inf"))

                victim = frames[distances.index(max(distances))]
                frames.remove(victim)
                frames.append(page)

        print(page, "->", frames, "Faults:", faults)

    return faults


if __name__ == "__main__":
    print("REFERENCE STRING:", REFERENCE)

    for count in [3, 4]:
        print("\n========== FRAMES:", count, "==========")
        print("FIFO faults:", fifo(REFERENCE, count))
        print("LRU faults:", lru(REFERENCE, count))
        print("Optimal faults:", optimal(REFERENCE, count))
