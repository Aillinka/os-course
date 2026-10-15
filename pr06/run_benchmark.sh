#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

echo "repeat,lock,threads,section,seconds,correct" > lock_repeats.csv

for repeat in 1 2 3; do
    echo "Running repeat $repeat"
    ./lock_benchmark | tail -n +2 | \
        awk -v n="$repeat" '{print n "," $0}' \
        >> lock_repeats.csv
done

echo "All benchmarks completed!"
