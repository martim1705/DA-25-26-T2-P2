#include <iostream>
#include "Parser.h"
#include "Web.h"
#include "Graph.h"
#include "InterferenceGraph.h"

using namespace std;

int main() {

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

    return 0;
}