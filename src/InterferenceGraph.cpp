#include "InterferenceGraph.h"
#include "Web.h"

bool interfere(const Web& webA, const Web& webB) {
    for (const auto& p1 : webA.points) {
        for (const auto& p2 : webB.points) {
            if (p1.line == p2.line) {
                return true;
            }
        }
    }
    return false;
}


Graph<Web> buildInterferenceGraph(const std::vector<Web>& webs) {
    Graph<Web> g;

    // add webs to Graph Node
    for (const auto& web : webs) {
        g.addVertex(web);
    }

    for (size_t i = 0; i < webs.size(); i++) {
        for (size_t j = i +1; j < webs.size(); j++) {
            if (interfere(webs[j], webs[i])) {
                g.addBidirectionalEdge(webs[i], webs[j], 0);
            }
        }
    }
    return g;
}
