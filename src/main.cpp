#include <iostream>
#include <string>
#include <vector>
#include "Parser.h"
#include "Web.h"
#include "Graph.h"
#include "InterferenceGraph.h"
#include "colorGraph.h"
#include "Output.h"

using namespace std;

namespace {
    bool isKnownAlgorithm(const string& algorithm) {
        return algorithm == "basic" || algorithm == "spilling" || algorithm == "splitting" || algorithm == "free";
    }

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
