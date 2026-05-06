cd cmake-build-debug || exit
# ==========================================
# PART 1: GENERATE CONFIGURATION FILES
# ==========================================
mkdir -p ../input/registers
mkdir -p ../output/tests

echo "Creating basic configs..."
for r in 1 2 3; do
    echo -e "registers: $r\nalgorithm: basic" > ../input/registers/reg_basic_${r}.txt
done

echo "Creating spilling configs..."
for r in 1 2 3; do
    for p in 1 2 3; do
        echo -e "registers: $r\nalgorithm: spilling, $p" > ../input/registers/reg_spill_r${r}_p${p}.txt
    done
done

echo "Creating splitting configs..."
for r in 1 2 3; do
    for p in 1 2 3; do
        echo -e "registers: $r\nalgorithm: splitting, $p" > ../input/registers/reg_split_r${r}_p${p}.txt
    done
done

# ==========================================
# PART 2: RUN ALL 126 PERMUTATIONS
# ==========================================
echo "Running test permutations..."

for range in 1 2 3 4 5 6; do
    # Run Basic tests (3 tests per range)
    for r in 1 2 3; do
        ./projeto2 -b ../input/ranges/ranges${range}.txt ../input/registers/reg_basic_${r}.txt ../output/tests/out_rg${range}_basic_r${r}.txt
    done

    # Run Spill tests (9 tests per range)
    for r in 1 2 3; do
        for p in 1 2 3; do
            ./projeto2 -b ../input/ranges/ranges${range}.txt ../input/registers/reg_spill_r${r}_p${p}.txt ../output/tests/out_rg${range}_spill_r${r}_p${p}.txt
        done
    done

    # Run Split tests (9 tests per range)
    for r in 1 2 3; do
        for p in 1 2 3; do
            ./projeto2 -b ../input/ranges/ranges${range}.txt ../input/registers/reg_split_r${r}_p${p}.txt ../output/tests/out_rg${range}_split_r${r}_p${p}.txt
        done
    done
done

echo "All 126 tests completed! Check the ../output/tests/ directory."

echo "Aggregating 126 test results..."
echo "MASTER TEST LOG" > ../output/all_results_merged.txt
echo "==========================================" >> ../output/all_results_merged.txt

for file in ../output/tests/*.txt; do
    echo "FILE: $(basename "$file")" >> ../output/all_results_merged.txt
    echo "------------------------------------------" >> ../output/all_results_merged.txt
    cat "$file" >> ../output/all_results_merged.txt
    echo -e "\n==========================================\n" >> ../output/all_results_merged.txt
done

echo "Done! You can now upload ../output/all_results_merged.txt"
