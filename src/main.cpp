/**
* @mainpage Register Allocation via Graph Coloring
 * * @section intro_sec Introduction
 * This project implements a compiler register allocator using graph coloring algorithms.
 * It supports four operational modes to map program variables (webs) to physical physical registers:
 * - **Basic**: Pure simplification-stack graph coloring.
 * - **Spilling**: Bounded removal of webs to memory using Hopcroft-Tarjan articulation point heuristics.
 * - **Splitting**: Bounded fragmentation of webs using temporal gap heuristics.
 * - **Free**: Exact DSATUR graph coloring with a polynomial heuristic fallback.
 * * @section author_sec Authors
 * Developed for DA 2025/2026.
 * - Artur Ferro
 * - João Leppänen
 * - Martim Ferreira
 * * @file main.cpp
 * @brief Command-line and interactive entry point for the register allocation tool.
 */
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "Parser.h"
#include "Web.h"
#include "Graph.h"
#include "InterferenceGraph.h"
#include "colorGraph.h"
#include "Output.h"

using namespace std;

namespace {
    /**
     * @brief Checks whether an algorithm name is supported by the allocator.
     * @param algorithm Algorithm name read from the configuration file.
     * @return true for basic, spilling, splitting or free.
     * @complexity O(1), because only four constant strings are compared.
     */
    bool isKnownAlgorithm(const string& algorithm) {
        return algorithm == "basic" || algorithm == "spilling" || algorithm == "splitting" || algorithm == "free";
    }

    /**
     * @brief Executes the full pipeline from input files to allocation output.
     *
     * The pipeline parses both inputs, validates user parameters, builds webs, builds the interference
     * graph, invokes the chosen coloring strategy and writes the final allocation file.
     *
     * @param rangesFile Path to the live-ranges input file.
     * @param registersFile Path to the register configuration file.
     * @param outputFile Path where the allocation output should be written.
     * @return true if allocation succeeded, false if input validation or allocation failed.
     * @complexity O(parse + W^2 * P^2 + allocation), where W is the number of webs and P is the
     * maximum number of points per web.  The allocation term depends on the selected algorithm.
     */
    bool runAllocation(const string& rangesFile, const string& registersFile, const string& outputFile) {
        RegisterConfig config = parseRegistersFile(registersFile);
        vector<LiveRange> ranges = parseRangesFile(rangesFile);

        if (ranges.empty()) {
            cerr << "Error: no valid live ranges were loaded.\n";
            return false;
        }
        if (config.numRegisters <= 0) {
            cerr << "Error: register count must be positive.\n";
            return false;
        }
        if (!isKnownAlgorithm(config.algorithm)) {
            cerr << "Error: unknown algorithm '" << config.algorithm << "'. Expected basic, spilling, splitting or free.\n";
            return false;
        }
        if ((config.algorithm == "spilling" || config.algorithm == "splitting") && config.parameter <= 0) {
            cerr << "Error: " << config.algorithm << " requires a positive numeric parameter.\n";
            return false;
        }

        vector<Web> webs = buildWebs(ranges);
        Graph<Web> graph = buildInterferenceGraph(webs);
        ColoringResult colorResult = colorGraphFunc(graph, config.numRegisters, config.algorithm, config.parameter);
        writeOutputToFile(outputFile, webs, colorResult);
        return colorResult.success;
    }

    /**
     * @brief Runs the simple interactive menu requested for the demo.
     * @return Process exit code.
     * @complexity Each allocation option has the complexity of runAllocation(); menu overhead is O(1).
     */
    int interactiveMenu() {
        string rangesFile;
        string registersFile;
        string outputFile;

        while (true) {
            cout << "\nRegister Allocation Tool\n";
            cout << "1. Run allocation\n";
            cout << "0. Exit\n";
            cout << "Option: ";

            int option = -1;
            if (!(cin >> option)) return 1;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (option == 0) return 0;
            if (option != 1) {
                cout << "Invalid option.\n";
                continue;
            }

            cout << "Ranges file: ";
            getline(cin, rangesFile);
            cout << "Registers file: ";
            getline(cin, registersFile);
            cout << "Output file: ";
            getline(cin, outputFile);

            bool success = runAllocation(rangesFile, registersFile, outputFile);
            cout << (success ? "Allocation completed.\n" : "Allocation finished with infeasibility/errors.\n");
        }
    }
}

/**
 * @brief Program entry point.
 *
 * With no arguments, launches the interactive menu.  In batch mode, expects the statement-compatible
 * syntax `-b <ranges.txt> <registers.txt> <allocation.txt>`.
 *
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument vector.
 * @return 0 on success, 1 on argument/menu errors and 2 when allocation is infeasible.
 * @complexity O(runAllocation) in batch mode and O(number_of_menu_runs * runAllocation) in interactive mode.
 */
int main(int argc, char* argv[]) {
    if (argc == 1) {
        return interactiveMenu();
    }

    if (string(argv[1]) != "-b" || argc != 5) {
        cerr << "Error: invalid arguments.\n";
        cerr << "Usage: " << argv[0] << " -b <ranges.txt> <registers.txt> <allocation.txt>\n";
        return 1;
    }

    const string rangesFile = argv[2];
    const string registersFile = argv[3];
    const string outputFile = argv[4];

    return runAllocation(rangesFile, registersFile, outputFile) ? 0 : 2;
}
