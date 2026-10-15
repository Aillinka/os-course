#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

MB="${1:-1024}"

echo "method,message_bytes,total_bytes,seconds,throughput_mib_s,us_per_message,result" > results.csv

for method in pipe mq shm socket; do
    for size in 64 4096 1048576; do
        echo "Testing $method, message size $size bytes, total $MB MiB"

        ./ipc_benchmark "$method" "$size" "$MB" \
            | tail -n 1 >> results.csv
    done
done

echo "All tests finished!"
cat results.csv
