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
#include <filesystem>
#include <fstream> // Added for file existence checking
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
     * @brief Builds an input path and smartly resolves it regardless of the working directory.
     *
     * It first checks the standard relative path. If that fails (e.g., the user is running
     * the program from inside the build/ directory), it falls back to looking one directory up.
     */
    string inputPathFromDefaultFolder(const string& folder, const string& filename) {
        string standardPath = folder + filename;

        // If it exists in the current directory (running from project root)
        if (std::filesystem::exists(standardPath)) {
            return standardPath;
        }

        // Fallback: check one directory up (running from build/ folder)
        string fallbackPath = "../" + folder + filename;
        if (std::filesystem::exists(fallbackPath)) {
            return fallbackPath;
        }

        // If neither exists, just return standard and let the later error-handlers catch it
        return standardPath;
    }

    /**
     * @brief Checks whether an algorithm name is supported by the allocator.
     * @param algorithm Algorithm name read from the configuration file.
     * @return true for basic, spilling, splitting or free.
     */
    bool isKnownAlgorithm(const string& algorithm) {
        return algorithm == "basic" || algorithm == "spilling" || algorithm == "splitting" || algorithm == "free";
    }

    /**
     * @brief Executes the full pipeline from input files to allocation output.
     *
     * @param rangesFile Path to the live-ranges input file.
     * @param registersFile Path to the register configuration file.
     * @param outputFile Path where the allocation output should be written.
     * @return true if allocation succeeded, false if input validation or allocation failed.
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
     * Option 2 is a convenience option that prepends default input folders.
     * Input validation traps the user until an existing file is provided or the input is valid.
     *
     * @return Process exit code.
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

            // Robust cin failure check: clears error, flushes buffer, and restarts menu loop
            if (!(cin >> option)) {
                cout << "Invalid option. Please enter a valid number.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // flush normal newline

            if (option == 0) return 0;
            if (option != 1 && option != 2) {
                cout << "Invalid option. Please select 0, 1, or 2.\n";
                continue;
            }

            // Robust loop for ranges file
            while (true) {
                if (option == 1) {
                    cout << "Ranges file path: ";
                    getline(cin, rangesFile);
                } else {
                    cout << "Ranges file name (inside " << DEFAULT_RANGES_FOLDER << "): ";
                    getline(cin, rangesFile);
                    rangesFile = inputPathFromDefaultFolder(DEFAULT_RANGES_FOLDER, rangesFile);
                }

                ifstream checkFile(rangesFile);
                if (checkFile.is_open()) {
                    break; // File exists, move on
                }
                cout << "Error: Could not open '" << rangesFile << "'. Please try again.\n";
            }

            // Robust loop for registers file
            while (true) {
                if (option == 1) {
                    cout << "Registers file path: ";
                    getline(cin, registersFile);
                } else {
                    cout << "Registers file name (inside " << DEFAULT_REGISTERS_FOLDER << "): ";
                    getline(cin, registersFile);
                    registersFile = inputPathFromDefaultFolder(DEFAULT_REGISTERS_FOLDER, registersFile);
                }

                ifstream checkFile(registersFile);
                if (checkFile.is_open()) {
                    break; // File exists, move on
                }
                cout << "Error: Could not open '" << registersFile << "'. Please try again.\n";
            }

            // Output file (no check needed, it will create it or fail safely in Output.cpp)
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