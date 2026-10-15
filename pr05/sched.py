import argparse
from collections import deque
from dataclasses import dataclass


@dataclass
class Process:
    pid: str
    arrival: int
    burst: int
    priority: int = 0


def load_processes(filename):
    processes = []
    with open(filename) as f:
        for line in f:
            if not line.strip() or line.startswith("#"):
                continue
            parts = line.split()
            if len(parts) != 4:
                raise ValueError("Expected PID arrival burst priority")
            pid, arrival, burst, priority = parts
            p = Process(pid, int(arrival), int(burst), int(priority))
            if p.arrival < 0 or p.burst <= 0:
                raise ValueError("Invalid arrival or burst")
            processes.append(p)
    if not processes or len({p.pid for p in processes}) != len(processes):
        raise ValueError("Empty input or duplicate PID")
    return processes


def simulate(processes, algorithm, quantum=3, boost=30):
    remaining = {p.pid: p.burst for p in processes}
    arrivals = sorted(processes, key=lambda p: (p.arrival, p.pid))
    timeline = []
    time = 0
    index = 0
    completed = 0
    total = len(processes)

    queues = [deque() for _ in range(3)]
    levels = {p.pid: 0 for p in processes}
    used = {p.pid: 0 for p in processes}
    running = None

    def add_arrivals():
        nonlocal index
        while index < total and arrivals[index].arrival <= time:
            pid = arrivals[index].pid
            queues[0].append(pid)
            index += 1

    def record(pid):
        if timeline and timeline[-1][2] == pid and timeline[-1][1] == time:
            start, _, name = timeline[-1]
            timeline[-1] = (start, time + 1, name)
        else:
            timeline.append((time, time + 1, pid))

    while completed < total:
        if algorithm == "srtf":
            ready = [
                p for p in processes
                if p.arrival <= time and remaining[p.pid] > 0
            ]
            if not ready:
                time += 1
                continue

            chosen = min(
                ready,
                key=lambda p: (remaining[p.pid], p.arrival, p.pid)
            )
            record(chosen.pid)
            remaining[chosen.pid] -= 1
            if remaining[chosen.pid] == 0:
                completed += 1
            time += 1
            continue

        add_arrivals()

        if time > 0 and time % boost == 0:
            waiting = []
            if running is not None:
                waiting.append(running)
                running = None
            for q in queues:
                waiting.extend(q)
                q.clear()
            for pid in waiting:
                levels[pid] = 0
                used[pid] = 0
                queues[0].append(pid)

        if running is None:
            for q in queues:
                if q:
                    running = q.popleft()
                    break

        if running is None:
            time += 1
            continue

        pid = running
        record(pid)
        remaining[pid] -= 1
        used[pid] += 1
        time += 1
        add_arrivals()

        if remaining[pid] == 0:
            completed += 1
            running = None
        elif used[pid] >= [2, 4, 8][levels[pid]]:
            levels[pid] = min(levels[pid] + 1, 2)
            used[pid] = 0
            queues[levels[pid]].append(pid)
            running = None
        elif any(queues[level] for level in range(levels[pid])):
            queues[levels[pid]].appendleft(pid)
            running = None

    return timeline


def metrics(processes, timeline):
    completion = {}
    first = {}
    for start, end, pid in timeline:
        completion[pid] = end
        first.setdefault(pid, start)

    waits = []
    turns = []
    responses = []
    for p in processes:
        turnaround = completion[p.pid] - p.arrival
        waits.append(turnaround - p.burst)
        turns.append(turnaround)
        responses.append(first[p.pid] - p.arrival)

    n = len(processes)
    return {
        "waiting": sum(waits) / n,
        "turnaround": sum(turns) / n,
        "response": sum(responses) / n,
        "switches": sum(
            timeline[i][2] != timeline[i - 1][2]
            for i in range(1, len(timeline))
        ),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--algo", choices=["mlfq", "srtf", "rr"],
                        default="mlfq")
    parser.add_argument("--quantum", type=int, default=3)
    parser.add_argument("--boost", type=int, default=30)
    parser.add_argument("file")
    args = parser.parse_args()

    if args.quantum <= 0 or args.boost <= 0:
        parser.error("quantum and boost must be positive")

    processes = load_processes(args.file)

    if args.algo == "rr":
        # RR is implemented separately below.
        timeline = simulate_rr(processes, args.quantum)
    else:
        timeline = simulate(processes, args.algo, boost=args.boost)

    result = metrics(processes, timeline)
    print("Algorithm:", args.algo.upper())
    print("Dataset:", args.file)
    print("Average waiting:", round(result["waiting"], 3))
    print("Average turnaround:", round(result["turnaround"], 3))
    print("Average response:", round(result["response"], 3))
    print("Context switches:", result["switches"])
    print("Gantt:", " | ".join(
        f"{start}-{end}:{pid}" for start, end, pid in timeline[:30]
    ))


def simulate_rr(processes, quantum):
    remaining = {p.pid: p.burst for p in processes}
    arrivals = sorted(processes, key=lambda p: (p.arrival, p.pid))
    queue = deque()
    timeline = []
    index = 0
    time = 0
    done = 0

    while done < len(processes):
        while index < len(arrivals) and arrivals[index].arrival <= time:
            queue.append(arrivals[index].pid)
            index += 1

        if not queue:
            time = arrivals[index].arrival
            continue

        pid = queue.popleft()
        duration = min(quantum, remaining[pid])
        timeline.append((time, time + duration, pid))
        time += duration
        remaining[pid] -= duration

        while index < len(arrivals) and arrivals[index].arrival <= time:
            queue.append(arrivals[index].pid)
            index += 1

        if remaining[pid]:
            queue.append(pid)
        else:
            done += 1

    return timeline


if __name__ == "__main__":
    main()
