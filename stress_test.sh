#!/usr/bin/env bash
set -u

# Portable stress test for the DA Project 2 allocator.
# Run from the project root after building, for example:
#   cmake -S . -B build
#   cmake --build build
#   ./stress_test.sh

EXECUTABLE="${1:-./build/projeto2}"
if [[ ! -x "$EXECUTABLE" ]]; then
  if [[ -x "./build2/projeto2" ]]; then
    EXECUTABLE="./build2/projeto2"
  elif [[ -x "./cmake-build-debug/projeto2" ]]; then
    EXECUTABLE="./cmake-build-debug/projeto2"
  else
    echo "Executable not found. Build first or pass the executable path as argument." >&2
    exit 1
  fi
fi

mkdir -p input/registers output/tests

for r in 1 2 3; do
  printf "registers: %s\nalgorithm: basic\n" "$r" > "input/registers/reg_basic_${r}.txt"
  printf "registers: %s\nalgorithm: free\n" "$r" > "input/registers/reg_free_${r}.txt"
  for p in 1 2 3; do
    printf "registers: %s\nalgorithm: spilling, %s\n" "$r" "$p" > "input/registers/reg_spill_r${r}_p${p}.txt"
    printf "registers: %s\nalgorithm: splitting, %s\n" "$r" "$p" > "input/registers/reg_split_r${r}_p${p}.txt"
  done
done

for range_file in input/ranges/ranges*.txt; do
  range_name="$(basename "$range_file" .txt)"

  for r in 1 2 3; do
    "$EXECUTABLE" -b "$range_file" "input/registers/reg_basic_${r}.txt" "output/tests/out_${range_name}_basic_r${r}.txt" || true
    "$EXECUTABLE" -b "$range_file" "input/registers/reg_free_${r}.txt" "output/tests/out_${range_name}_free_r${r}.txt" || true
  done

  for r in 1 2 3; do
    for p in 1 2 3; do
      "$EXECUTABLE" -b "$range_file" "input/registers/reg_spill_r${r}_p${p}.txt" "output/tests/out_${range_name}_spill_r${r}_p${p}.txt" || true
      "$EXECUTABLE" -b "$range_file" "input/registers/reg_split_r${r}_p${p}.txt" "output/tests/out_${range_name}_split_r${r}_p${p}.txt" || true
    done
  done
done

{
  echo "MASTER TEST LOG"
  echo "=========================================="
  for file in output/tests/*.txt; do
    echo "FILE: $(basename "$file")"
    echo "------------------------------------------"
    cat "$file"
    printf "\n==========================================\n\n"
  done
} > output/all_results_merged.txt

echo "Stress tests completed. See output/all_results_merged.txt"
