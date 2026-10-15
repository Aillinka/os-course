# Practical Work 5 — CPU Scheduling

**Course:** Operating Systems
**Level:** C
**Environment:** Ubuntu Linux

## Objective

To investigate CPU scheduling algorithms and
compare simulated scheduling with real Linux
process scheduling.

## Task 1 — MLFQ Simulator

A three-level Multilevel Feedback Queue
scheduler was implemented in Python.

Queue configuration:

- Q0: quantum 2
- Q1: quantum 4
- Q2: quantum 8

Processes are demoted after using their
full time quantum.

A priority boost occurs every 30 time units.

MLFQ is compared with SRTF using three datasets.

## Execution

    python3 sched.py --algo mlfq set1.txt
    python3 sched.py --algo srtf set1.txt
    python3 sched.py --algo rr --quantum 3 set1.txt
    python3 compare.py

## Results

Simulation results are stored in comparison.csv.

The comparison graph is comparison.png.

SRTF uses knowledge of remaining CPU burst time.

MLFQ does not require this information and
adapts priorities based on process behavior.

## Task 2 — Linux Nice Experiment

Four CPU-bound processes were started with
nice values 0, 5, 10, and 15.

The processes competed for one logical CPU.

CPU execution time was measured using
/proc/PID/schedstat.

Results are stored in nice_results.csv.

## Task 3 — Scheduling Policies

The current scheduling policy was inspected
using chrt -p.

Information is stored in scheduling_info.txt.

SCHED_FIFO was studied conceptually.
A real-time workload was not executed.

## Limitations

The simulation uses discrete time units and
simplified scheduling rules.

The Linux nice experiment is a short sample
and may be affected by startup timing.

The SCHED_FIFO execution experiment remains
incomplete.

The supplied reference values for set1
should be checked against the stated
waiting and turnaround formulas.

## Conclusion

MLFQ uses multiple priority queues and
feedback to improve responsiveness without
knowing process burst times in advance.

SRTF can minimize average waiting time under
ideal assumptions but requires knowledge of
remaining execution time.

Linux nice values influence the distribution
of CPU time among competing processes.

Real-time scheduling requires careful resource
management to prevent starvation of ordinary
tasks.
