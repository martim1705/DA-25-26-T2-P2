/**
 * @file InterferenceGraph.cpp
 * @brief Implementation of live-web interference tests and graph construction.
 */

#include "InterferenceGraph.h"

namespace {
    /**
     * @brief Checks the special boundary case where two webs touch but do not interfere.
     *
     * If one point only starts and the other only ends on the same line, the ending value can be
     * read before the new value is written, so both values do not need different registers.
     *
     * @param a First program point.
     * @param b Second program point.
     * @return true if the two points form a start/end boundary.
     * @complexity O(1).
     */
    bool isStartEndBoundary(const ProgramPoint& a, const ProgramPoint& b) {
        const bool aOnlyStarts = a.isStart && !a.isEnd;
        const bool aOnlyEnds = a.isEnd && !a.isStart;
        const bool bOnlyStarts = b.isStart && !b.isEnd;
        const bool bOnlyEnds = b.isEnd && !b.isStart;
        return (aOnlyStarts && bOnlyEnds) || (aOnlyEnds && bOnlyStarts);
    }
}

bool interfere(const Web& webA, const Web& webB) {
    for (const auto& p1 : webA.points) {
        for (const auto& p2 : webB.points) {
            if (p1.line != p2.line) continue;

            // Same line usually means both values are live at the same program
            // point.  The exception is the classic "x = y" boundary: one value
            // is read for the last time while the other is defined for the
            // first time, so the same register can be reused safely.
            if (!isStartEndBoundary(p1, p2)) {
                return true;
            }
        }
    }
    return false;
}

Graph<Web> buildInterferenceGraph(const std::vector<Web>& webs) {
    Graph<Web> graph;
    for (const auto& web : webs) {
        graph.addVertex(web);
    }

    for (size_t i = 0; i < webs.size(); i++) {
        for (size_t j = i + 1; j < webs.size(); j++) {
            if (interfere(webs[i], webs[j])) {
                graph.addBidirectionalEdge(webs[i], webs[j], 0);
            }
        }
    }
    return graph;
}
