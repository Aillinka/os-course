import sys

def load_data(filename):
    with open(filename) as f:
        rows = [
            list(map(int, line.split()))
            for line in f if line.strip()
        ]

    if len(rows) != 11 or any(len(row) != 3 for row in rows):
        raise ValueError("Invalid bank file format")

    available = rows[0]
    allocation = rows[1:6]
    maximum = rows[6:11]

    return available, allocation, maximum


def check_safety(available, allocation, maximum):
    n = len(allocation)
    m = len(available)

    need = [
        [maximum[i][j] - allocation[i][j] for j in range(m)]
        for i in range(n)
    ]

    work = available.copy()
    finish = [False] * n
    sequence = []

    while len(sequence) < n:
        found = False

        for i in range(n):
            if finish[i]:
                continue

            if all(need[i][j] <= work[j] for j in range(m)):
                for j in range(m):
                    work[j] += allocation[i][j]

                finish[i] = True
                sequence.append(i)
                found = True

        if not found:
            return False, sequence

    return True, sequence


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 banker.py bank1.txt")
        sys.exit(1)

    available, allocation, maximum = load_data(sys.argv[1])

    safe, sequence = check_safety(available, allocation, maximum)

    print("File:", sys.argv[1])
    print("Available:", available)

    if safe:
        print("SAFE")
        print("Safe sequence:", " -> ".join(f"P{i}" for i in sequence))
    else:
        print("UNSAFE")
        print("Completed processes:", sequence)
