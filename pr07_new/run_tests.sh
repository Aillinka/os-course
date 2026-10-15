
#!/bin/bash

set -e

TOTAL_MB=${1:-4}
OUTPUT="results_${TOTAL_MB}MB.csv"

echo "Message_Size,Mechanism,Speed_MB_s,Latency_us" > "$OUTPUT"

for SIZE in 64 4096 1048576
do
    echo "================================="
    echo "Message size: $SIZE bytes"
    echo "Total data: $TOTAL_MB MB"
    echo "================================="

    ./ipc_benchmark "$SIZE" "$TOTAL_MB" | \
    awk -v size="$SIZE" '
    /PIPE:/ {
        print size ",PIPE," $2 "," $4
    }
    /SOCKET:/ {
        print size ",SOCKET," $2 "," $4
    }' >> "$OUTPUT"

    ./ipc_extra "$SIZE" "$TOTAL_MB" | \
    awk -v size="$SIZE" '
    /MESSAGE QUEUE:/ {
        print size ",MESSAGE_QUEUE," $3 "," $5
    }
    /SHARED MEMORY:/ {
        print size ",SHARED_MEMORY," $3 "," $5
    }' >> "$OUTPUT"

    echo "Completed: $SIZE bytes"
done

echo
echo "Results saved to $OUTPUT"
cat "$OUTPUT"
