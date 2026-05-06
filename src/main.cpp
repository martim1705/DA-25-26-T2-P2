#include <iostream>
#include "Parser.h"
#include "Web.h"
#include "Graph.h"
#include "InterferenceGraph.h"
#include "colorGraph.h"
#include "Output.h"
using namespace std;

int main(int argc, char* argv[]) {
    // Default file paths for testing via your IDE without arguments
    string rangesFile = "../input/ranges.txt";
    string registersFile = "../input/registers.txt";
    string outputFile = "../output/allocation.txt";

    // Handle command line arguments for batch mode [T1.1 requirement]
    if (argc > 1) {
        if (string(argv[1]) == "-b") {
            // If -b is used, there MUST be exactly 5 arguments total:
            // [0]prog [1]-b [2]ranges [3]registers [4]output
            if (argc != 5) {
                cerr << "Error: Incorrect number of arguments for batch mode.\n";
                cerr << "Expected Usage: " << argv[0] << " -b <ranges.txt> <registers.txt> <output.txt>\n";
                return 1; // Exit with error code
            }
            rangesFile = argv[2];
            registersFile = argv[3];
            outputFile = argv[4];
        } else {
            // Any other flag or usage is rejected
            cerr << "Error: Invalid argument passed. Only batch mode (-b) is currently supported via CLI.\n";
            cerr << "Expected Usage: " << argv[0] << " -b <ranges.txt> <registers.txt> <output.txt>\n";
            return 1; // Exit with error code
        }
    }
    // 1. Parse Input Data
    RegisterConfig config = parseRegistersFile(registersFile);
    vector<LiveRange> ranges = parseRangesFile(rangesFile);

    // 2. Build Data Structures
    vector<Web> webs = buildWebs(ranges);
    Graph<Web> g = buildInterferenceGraph(webs);

    // 3. Execute Allocation (Graph Coloring)
    ColoringResult colorResult = colorGraphFunc(g, config.numRegisters, config.algorithm, config.parameter);

    /* Extra error detection...
    if (!colorResult.success) {
        cerr << "Assignment not feasible for the given constraints.\n";
    }
    */

    // 4. Output to File
    writeOutputToFile(outputFile, webs, colorResult);

    return 0;
}

/*
    // testing parser
    RegisterConfig config = parseRegistersFile("../input/registers");
    vector<LiveRange> ranges = parseRangesFile("../input/ranges");

    for (const LiveRange& range : ranges) {
        cout << "Variable: " << range.variable << endl;

        for (const ProgramPoint& p : range.points) {
            cout << p.line;
            if (p.isStart) cout << "+";
            if (p.isEnd) cout << "-";
            cout << " ";
        }

        cout << endl << endl;
    }
    cout << "Registers: " << config.numRegisters << endl;
    cout << "Algorithm: " << config.algorithm << endl;
    cout << "Parameter: " << config.parameter << endl;


// testing buildWebs
    std::vector<Web> webs = buildWebs(ranges);

    for (const Web& w : webs) {
        std::cout << "Web " << w.id << " (" << w.variable << "): ";

        for (const ProgramPoint& p : w.points) {
            std::cout << p.line;
            if (p.isStart) std::cout << "+";
            if (p.isEnd) std::cout << "-";
            std::cout << " ";
        }

        std::cout << std::endl;
    }


// teste InterferenceGraph

    Graph<Web> g = buildInterferenceGraph(webs);

    for (auto v : g.getVertexSet()) {
        std::cout << "Web " << v->getInfo().id << " -> ";

        for (auto e : v->getAdj()) {
            std::cout << "Web " << e->getDest()->getInfo().id << " ";
        }

        std::cout << std::endl;
    }
*/