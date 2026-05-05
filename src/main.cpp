#include <iostream>
#include "Parser.h"
#include "Web.h"
#include "Graph.h"
#include "InterferenceGraph.h"
#include "colorGraph.h"

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

//Teste BasicColorGraph (T2.1)

    const ColoringResult colorResult = colorGraphFunc(g, config.numRegisters, config.algorithm,config.parameter);

    std::cout << "# Total number of registers used, followed by assignment to webs\n";

    if (colorResult.success) {
        int maxColorUsed = -1;
        for (const auto& webID_color : colorResult.colorOfWeb) {
            if (webID_color.second > maxColorUsed) {
                maxColorUsed = webID_color.second;
            }
        }
        const int registersUsed = maxColorUsed + 1;

        std::cout << "registers: " << registersUsed << "\n";

        /*for (const auto& web : webs) {
            const int assignedColor = colorResult.colorOfWeb.at(web.id);
            std::cout << "r" << assignedColor << ": web" << web.id << "\n";
        }
        */

        for (const auto& webID_color:colorResult.colorOfWeb) {
            int webId = webID_color.first;
            int assignedColor = webID_color.second;

            if (assignedColor==-1) {
                std::cout << "M: web" << webId << "\n";
            }
            else {
                std::cout << "r" << assignedColor << ": web" << webId << "\n";
            }
        }

    } else {
        std::cout << "registers: 0\n";

        for (const auto& web : webs) {
            std::cout << "M: web" << web.id << "\n";
        }
    }


    return 0;
}