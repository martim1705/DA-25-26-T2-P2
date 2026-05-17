/**
* @mainpage Register Allocation via Graph Coloring
 *
 * @section intro_sec Introduction
 * This project implements a compiler register allocator using graph coloring algorithms.
 * It supports four operational modes to map program variables (webs) to physical registers:
 * - **Basic**: Pure simplification-stack graph coloring. Nodes with a degree lower than the number of available registers are pushed to a stack for later coloring. If the graph cannot be simplified further (all remaining nodes equal or exceed the register count), the allocation immediately aborts as infeasible.
 * - **Spilling**: Bounded removal of webs to memory. If basic allocation fails, it selectively spills up to K webs using structural heuristics (Hopcroft-Tarjan articulation points, falling back to maximum degree) and iteratively retries the basic allocator.
 * - **Splitting**: Bounded fragmentation of webs. If basic allocation fails, it uses a greedy search to evaluate all possible split points across all webs. It applies up to K splits, always choosing the one that most reduces total interference edges and maximum graph degree, iteratively retrying the basic allocator.
 * - **Free**: Exact mathematical coloring using DSATUR backtracking, with a fallback to polynomial-time structural spilling heuristics if an exact coloring is not found.
 *
 * @section author_sec Authors
 * Developed for DA 2025/2026.
 * - Artur Ferro
 * - João Leppänen
 * - Martim Ferreira
 *
 * @file main.cpp
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
    /** @brief Default folder used by the convenience menu option for live-range input files. */
    const string DEFAULT_RANGES_FOLDER = "input/ranges/";

    /** @brief Default folder used by the convenience menu option for register-configuration input files. */
    const string DEFAULT_REGISTERS_FOLDER = "input/registers/";

    /**
     * @brief Builds an input path by prepending a fixed folder to a file name.
     *
     * This is intentionally simple and hardcoded for the demo menu: the user writes only
     * something like `ranges1.txt` or `registers1.txt`, and the function resolves it under
     * the corresponding input subfolder.
     *
     * @param folder Fixed input folder ending in a path separator.
     * @param filename File name typed by the user.
     * @return Full relative path to the input file.
     * @complexity O(F), where F is the file-name length.
     */
    string inputPathFromDefaultFolder(const string& folder, const string& filename) {
        return folder + filename;
    }

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
     *
     * Option 1 keeps the original behavior and asks for complete paths.
     * Option 2 is a convenience option that prepends `input/ranges/` and `input/registers/`
     * to the file names typed by the user.
     *
     * @return Process exit code.
     * @complexity Each allocation option has the complexity of runAllocation(); menu overhead is O(1).
     */
    int interactiveMenu() {
        string rangesFile;
        string registersFile;
        string outputFile;

        while (true) {
            cout << "\nRegister Allocation Tool\n";
            cout << "1. Run allocation with explicit paths\n";
            cout << "2. Run allocation from default input folders\n";
            cout << "0. Exit\n";
            cout << "Option: ";

            int option = -1;
            if (!(cin >> option)) return 1;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (option == 0) return 0;
            if (option != 1 && option != 2) {
                cout << "Invalid option.\n";
                continue;
            }

            if (option == 1) {
                cout << "Ranges file path: ";
                getline(cin, rangesFile);
                cout << "Registers file path: ";
                getline(cin, registersFile);
            } else {
                cout << "Ranges file name (inside " << DEFAULT_RANGES_FOLDER << "): ";
                getline(cin, rangesFile);
                cout << "Registers file name (inside " << DEFAULT_REGISTERS_FOLDER << "): ";
                getline(cin, registersFile);
                rangesFile = inputPathFromDefaultFolder(DEFAULT_RANGES_FOLDER, rangesFile);
                registersFile = inputPathFromDefaultFolder(DEFAULT_REGISTERS_FOLDER, registersFile);
            }

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
