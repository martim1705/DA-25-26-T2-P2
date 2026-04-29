#include <iostream>
#include "Parser.h"
using namespace std;

int main() {
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
    return 0;
}