#!/usr/bin/env bash
set -euo pipefail

make clean
make all

output="$(./maxint)"
echo "$output"

array_line="$(printf '%s\n' "$output" | sed -n '1p')"
max_line="$(printf '%s\n' "$output" | sed -n '2p')"

case "$array_line" in
  Array:\ *) ;;
  *) echo "Invalid output: missing array line"; exit 1 ;;
esac

case "$max_line" in
  Max:\ *) ;;
  *) echo "Invalid output: missing max line"; exit 1 ;;
esac

numbers="${array_line#Array: }"
reported_max="$(printf '%s\n' "$max_line" | awk '{print $2}')"
calculated_max="$(printf '%s\n' "$numbers" | tr ' ' '\n' | sort -n | tail -n1)"

if [ "$reported_max" != "$calculated_max" ]; then
  echo "Test failed: reported max $reported_max, calculated max $calculated_max"
  exit 1
fi

echo "Test passed"
