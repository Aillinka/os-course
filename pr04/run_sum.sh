#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

echo "threads,time_seconds,sum" > sum_results.csv

for n in 1 2 4 8 16; do
    for repeat in 1 2 3; do
        echo "Threads=$n Repeat=$repeat"
        ./parallel_sum "$n" >> sum_results.csv
    done
done

echo "Benchmark finished!"
