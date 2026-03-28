#!/usr/bin/env bash
set -euo pipefail

PORT=8080
BASE_URL="http://localhost:${PORT}"
SERVER_PID=""

cleanup() {
    if [ -n "$SERVER_PID" ]; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

make clean
make all

./maxint &
SERVER_PID=$!

# Wait up to 5 seconds for the server to become ready
for i in $(seq 1 10); do
    if curl -sf "${BASE_URL}/healthz" >/dev/null 2>&1; then
        break
    fi
    sleep 0.5
done

echo "=== Testing /healthz ==="
health=$(curl -sf "${BASE_URL}/healthz")
[ "$health" = "OK" ] || { echo "FAIL: /healthz returned: $health"; exit 1; }
echo "OK"

echo "=== Testing /compute ==="
output=$(curl -sf "${BASE_URL}/compute")
echo "$output"

array_line=$(printf '%s\n' "$output" | sed -n '1p')
max_line=$(printf '%s\n' "$output" | sed -n '2p')

case "$array_line" in
  Array:\ *) ;;
  *) echo "FAIL: missing Array: line"; exit 1 ;;
esac

case "$max_line" in
  Max:\ *) ;;
  *) echo "FAIL: missing Max: line"; exit 1 ;;
esac

numbers="${array_line#Array: }"
reported_max=$(printf '%s\n' "$max_line" | awk '{print $2}')
calculated_max=$(echo "$numbers" | tr ' ' '\n' | sort -n | tail -n1)

if [ "$reported_max" != "$calculated_max" ]; then
    echo "FAIL: reported max $reported_max != calculated max $calculated_max"
    exit 1
fi

echo "=== Testing /metrics ==="
metrics=$(curl -sf "${BASE_URL}/metrics")
echo "$metrics" | grep -q "maxint_requests_total" || { echo "FAIL: missing maxint_requests_total"; exit 1; }
echo "$metrics" | grep -q "maxint_computations_total" || { echo "FAIL: missing maxint_computations_total"; exit 1; }
echo "$metrics" | grep -q "maxint_last_max_value" || { echo "FAIL: missing maxint_last_max_value"; exit 1; }
echo "$metrics" | grep -q "maxint_up 1" || { echo "FAIL: missing maxint_up 1"; exit 1; }

echo "=== All tests passed ==="
