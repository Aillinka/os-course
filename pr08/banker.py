import sys

allocation = [
    [0, 1, 0],
    [2, 0, 0],
    [3, 0, 2],
    [2, 1, 1],
    [0, 0, 2],
]

maximum = [
    [7, 5, 3],
    [3, 2, 2],
    [9, 0, 2],
    [2, 2, 2],
    [4, 3, 3],
]


def check_safety(available):
    n = len(allocation)
    m = len(available)

    need = [
        [maximum[i][j] - allocation[i][j]
         for j in range(m)]
        for i in range(n)
    ]

    work = available.copy()
    finished = [False] * n
    sequence = []

    while len(sequence) < n:
        progress = False

        for i in range(n):
            if finished[i]:
                continue

            if all(need[i][j] <= work[j]
                   for j in range(m)):
                for j in range(m):
                    work[j] += allocation[i][j]

                finished[i] = True
                sequence.append(f"P{i}")
                progress = True

        if not progress:
            return None

    return sequence


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 banker.py bank1|bank2")
        sys.exit(1)

    if sys.argv[1] == "bank1":
        available = [3, 3, 2]
    elif sys.argv[1] == "bank2":
        available = [0, 1, 0]
    else:
        print("Unknown dataset")
        sys.exit(1)

    result = check_safety(available)

    print("Available:", available)

    if result is None:
        print("UNSAFE")
    else:
        print("SAFE")
        print("Sequence:", " -> ".join(result))
