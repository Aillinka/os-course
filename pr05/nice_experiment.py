import csv
import os
import subprocess
import sys
import time

cpu = min(os.sched_getaffinity(0))
nice_values = [0, 5, 10, 15]
processes = []

worker_code = """
import time
end = time.monotonic() + 4
x = 0
while time.monotonic() < end:
    x = (x * 17 + 3) % 1000003
"""

def setup(nice):
    def configure():
        os.sched_setaffinity(0, {cpu})
        os.nice(nice)
    return configure

try:
    for value in nice_values:
        process = subprocess.Popen(
            [sys.executable, "-c", worker_code],
            preexec_fn=setup(value),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        processes.append((value, process))

    time.sleep(2)

    rows = []
    for value, process in processes:
        path = f"/proc/{process.pid}/schedstat"
        with open(path) as f:
            cpu_ns, wait_ns, slices = map(int, f.read().split())

        rows.append({
            "nice": value,
            "pid": process.pid,
            "cpu_ms": round(cpu_ns / 1e6, 3),
            "wait_ms": round(wait_ns / 1e6, 3),
            "slices": slices
        })

    with open("nice_results.csv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)

    for row in rows:
        print(row)

finally:
    for _, process in processes:
        if process.poll() is None:
            process.terminate()

    for _, process in processes:
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()

print("Experiment completed. All child processes cleaned up.")
