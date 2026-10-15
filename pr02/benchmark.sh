#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

echo "method,buffer_size,time_seconds" > results.csv

for method in rw stdio; do
    for size in 1 64 4096 65536; do
        for repeat in 1 2 3; do
            echo "Testing $method | buffer=$size | repeat=$repeat"

            ./copy_benchmark \
                "$method" "$size" \
                test_100mb.bin copied.bin >> results.csv

            cmp -s test_100mb.bin copied.bin || {
                echo "ERROR: files are different"
                exit 1
            }
        done
    done
done

for repeat in 1 2 3; do
    echo "Testing sendfile | repeat=$repeat"

    ./copy_benchmark \
        sendfile 65536 \
        test_100mb.bin copied.bin >> results.csv

    cmp -s test_100mb.bin copied.bin || exit 1
done

echo "All experiments completed!"
