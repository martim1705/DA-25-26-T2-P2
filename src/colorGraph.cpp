#include "colorGraph.h"
#include "InterferenceGraph.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    struct AllocationAttempt {
        bool success = false;
        std::unordered_map<int, int> colors;
        int registersUsed = 0;
    };

    struct SplitCandidate {
        bool valid = false;
        int webId = -1;
        size_t splitIndex = 0;
        int resultingEdges = 0;
        int resultingMaxDegree = 0;
        int originalDegree = 0;
    };

    std::vector<Web> graphWebs(const Graph<Web>& graph) {
        std::vector<Web> webs;
        for (auto vertex : graph.getVertexSet()) {
            webs.push_back(vertex->getInfo());
        }
        std::sort(webs.begin(), webs.end(), [](const Web& a, const Web& b) { return a.id < b.id; });
        return webs;
    }

    int maxWebId(const std::vector<Web>& webs) {
        int maxId = -1;
        for (const auto& web : webs) maxId = std::max(maxId, web.id);
        return maxId;
    }

    int edgeCount(const Graph<Web>& graph) {
        int directed = 0;
        for (auto vertex : graph.getVertexSet()) {
            directed += static_cast<int>(vertex->getAdj().size());
        }
        return directed / 2;
    }

    int maxDegree(const Graph<Web>& graph) {
        int best = 0;
        for (auto vertex : graph.getVertexSet()) {
            best = std::max(best, static_cast<int>(vertex->getAdj().size()));
        }
        return best;
    }

    int webLength(const Web& web) {
        return static_cast<int>(web.points.size());
    }

    Graph<Web> graphWithoutSpilled(const std::vector<Web>& webs, const std::set<int>& spilled) {
        std::vector<Web> kept;
        for (const auto& web : webs) {
            if (!spilled.count(web.id)) kept.push_back(web);
        }
        return buildInterferenceGraph(kept);
    }

    bool hasInterferenceConflict(const Graph<Web>& graph, const std::unordered_map<int, int>& colors) {
        for (auto vertex : graph.getVertexSet()) {
            int id = vertex->getInfo().id;
            auto it = colors.find(id);
            if (it == colors.end() || it->second < 0) continue;

            for (auto edge : vertex->getAdj()) {
                int otherId = edge->getDest()->getInfo().id;
                auto otherIt = colors.find(otherId);
                if (otherIt != colors.end() && otherIt->second >= 0 && otherIt->second == it->second) {
                    return true;
                }
            }
        }
        return false;
    }

    AllocationAttempt simplifyAndColor(const Graph<Web>& originalGraph, int colorCount) {
        AllocationAttempt attempt;
        if (colorCount <= 0) return attempt;

        Graph<Web> tempGraph(originalGraph);
        std::stack<Web> stack;

        while (tempGraph.getNumVertex() > 0) {
            Vertex<Web>* best = nullptr;
            for (auto vertex : tempGraph.getVertexSet()) {
                int degree = static_cast<int>(vertex->getAdj().size());
                if (degree < colorCount && (best == nullptr || degree < static_cast<int>(best->getAdj().size()) ||
                    (degree == static_cast<int>(best->getAdj().size()) && vertex->getInfo().id < best->getInfo().id))) {
                    best = vertex;
                }
            }

            if (best == nullptr) {
                // Optimistic coloring: when the simplification rule gets stuck,
                // push the highest-degree node anyway.  If the graph is truly not
                // colorable with this color count, the assignment phase will fail
                // cleanly when no register is available.
                int bestDegree = -1;
                for (auto vertex : tempGraph.getVertexSet()) {
                    int degree = static_cast<int>(vertex->getAdj().size());
                    if (degree > bestDegree || (degree == bestDegree && vertex->getInfo().id < best->getInfo().id)) {
                        best = vertex;
                        bestDegree = degree;
                    }
                }
            }

            Web selected = best->getInfo();
            stack.push(selected);
            tempGraph.removeVertex(selected);
        }

        while (!stack.empty()) {
            Web current = stack.top();
            stack.pop();

            std::vector<bool> forbidden(colorCount, false);
            const Vertex<Web>* vertex = originalGraph.findVertex(current);
            if (vertex == nullptr) return AllocationAttempt{};

            for (auto edge : vertex->getAdj()) {
                int neighborId = edge->getDest()->getInfo().id;
                auto it = attempt.colors.find(neighborId);
                if (it != attempt.colors.end() && it->second >= 0 && it->second < colorCount) {
                    forbidden[it->second] = true;
                }
            }

            int chosenColor = -1;
            for (int color = 0; color < colorCount; color++) {
                if (!forbidden[color]) {
                    chosenColor = color;
                    break;
                }
            }

            if (chosenColor == -1) {
                return AllocationAttempt{};
            }
            attempt.colors[current.id] = chosenColor;
            attempt.registersUsed = std::max(attempt.registersUsed, chosenColor + 1);
        }

        if (hasInterferenceConflict(originalGraph, attempt.colors)) {
            return AllocationAttempt{};
        }
        attempt.success = true;
        return attempt;
    }

    AllocationAttempt tryBasicUpTo(const Graph<Web>& graph, int maxRegisters) {
        for (int colors = 1; colors <= maxRegisters; colors++) {
            AllocationAttempt attempt = simplifyAndColor(graph, colors);
            if (attempt.success) return attempt;
        }
        return AllocationAttempt{};
    }

    ColoringResult makeResult(bool success, const std::vector<Web>& webs, const std::unordered_map<int, int>& colors) {
        ColoringResult result;
        result.success = success;
        result.finalWebs = webs;
        result.colorOfWeb = colors;
        return result;
    }

    ColoringResult basicAllocation(const std::vector<Web>& webs, int maxRegisters) {
        Graph<Web> graph = buildInterferenceGraph(webs);
        AllocationAttempt attempt = tryBasicUpTo(graph, maxRegisters);
        if (!attempt.success) return makeResult(false, webs, {});
        return makeResult(true, webs, attempt.colors);
    }

    std::vector<Web> removeWebs(const std::vector<Web>& webs, const std::set<int>& idsToRemove) {
        std::vector<Web> kept;
        for (const auto& web : webs) {
            if (!idsToRemove.count(web.id)) kept.push_back(web);
        }
        return kept;
    }

    ColoringResult trySpillSet(const std::vector<Web>& webs, int maxRegisters, const std::set<int>& spilled) {
        std::vector<Web> kept = removeWebs(webs, spilled);
        Graph<Web> reducedGraph = buildInterferenceGraph(kept);
        AllocationAttempt attempt;

        if (kept.empty()) {
            attempt.success = true;
        } else {
            attempt = tryBasicUpTo(reducedGraph, maxRegisters);
        }

        if (!attempt.success) return makeResult(false, webs, {});

        for (int id : spilled) {
            attempt.colors[id] = -1;
        }
        return makeResult(true, webs, attempt.colors);
    }

    std::vector<Web> spillCandidatesByHeuristic(const Graph<Web>& graph) {
        std::vector<Web> candidates = graphWebs(graph);
        std::sort(candidates.begin(), candidates.end(), [&graph](const Web& a, const Web& b) {
            int degreeA = 0;
            int degreeB = 0;
            if (graph.findVertex(a) != nullptr) degreeA = static_cast<int>(graph.findVertex(a)->getAdj().size());
            if (graph.findVertex(b) != nullptr) degreeB = static_cast<int>(graph.findVertex(b)->getAdj().size());
            if (degreeA != degreeB) return degreeA > degreeB;
            if (webLength(a) != webLength(b)) return webLength(a) > webLength(b);
            return a.id < b.id;
        });
        return candidates;
    }

    ColoringResult spillingAllocation(const std::vector<Web>& webs, int maxRegisters, int spillBudget) {
        ColoringResult basic = basicAllocation(webs, maxRegisters);
        if (basic.success) return basic;

        Graph<Web> originalGraph = buildInterferenceGraph(webs);
        std::vector<Web> ordered = spillCandidatesByHeuristic(originalGraph);
        const int n = static_cast<int>(ordered.size());
        const int maxSpills = std::min(spillBudget, n);
        const long long combinationLimit = 20000;

        for (int targetSpills = 1; targetSpills <= maxSpills; targetSpills++) {
            long long visited = 0;
            std::set<int> current;
            ColoringResult best;

            std::function<bool(int, int)> dfs = [&](int start, int left) -> bool {
                if (++visited > combinationLimit) return false;
                if (left == 0) {
                    ColoringResult attempt = trySpillSet(webs, maxRegisters, current);
                    if (attempt.success) {
                        best = attempt;
                        return true;
                    }
                    return false;
                }
                for (int i = start; i <= n - left; i++) {
                    current.insert(ordered[i].id);
                    if (dfs(i + 1, left - 1)) return true;
                    current.erase(ordered[i].id);
                }
                return false;
            };

            if (dfs(0, targetSpills) && best.success) return best;

            // If exhaustive search was capped, use the same ordering greedily.
            if (visited > combinationLimit) {
                std::set<int> greedy;
                for (int i = 0; i < targetSpills; i++) greedy.insert(ordered[i].id);
                ColoringResult attempt = trySpillSet(webs, maxRegisters, greedy);
                if (attempt.success) return attempt;
            }
        }

        return makeResult(false, webs, {});
    }

    void sortWebPoints(Web& web) {
        std::sort(web.points.begin(), web.points.end(), [](const ProgramPoint& a, const ProgramPoint& b) {
            return a.line < b.line;
        });
    }

    bool splitWeb(const Web& original, size_t splitIndex, int newId, Web& left, Web& right) {
        if (original.points.size() < 2 || splitIndex >= original.points.size() - 1) return false;

        left = original;
        right = original;
        left.points.assign(original.points.begin(), original.points.begin() + static_cast<long>(splitIndex) + 1);
        right.points.assign(original.points.begin() + static_cast<long>(splitIndex) + 1, original.points.end());
        right.id = newId;
        sortWebPoints(left);
        sortWebPoints(right);
        return true;
    }

    std::vector<Web> replaceWithSplit(const std::vector<Web>& webs, int webId, size_t splitIndex, int newId) {
        std::vector<Web> result;
        for (const auto& web : webs) {
            if (web.id != webId) {
                result.push_back(web);
                continue;
            }
            Web left, right;
            if (splitWeb(web, splitIndex, newId, left, right)) {
                result.push_back(left);
                result.push_back(right);
            } else {
                result.push_back(web);
            }
        }
        std::sort(result.begin(), result.end(), [](const Web& a, const Web& b) { return a.id < b.id; });
        return result;
    }

    SplitCandidate chooseSplitCandidate(const std::vector<Web>& webs) {
        Graph<Web> currentGraph = buildInterferenceGraph(webs);
        SplitCandidate best;
        int nextId = maxWebId(webs) + 1;

        for (const auto& web : webs) {
            if (web.points.size() < 2) continue;
            int originalDegree = 0;
            if (currentGraph.findVertex(web) != nullptr) {
                originalDegree = static_cast<int>(currentGraph.findVertex(web)->getAdj().size());
            }

            for (size_t splitIndex = 0; splitIndex + 1 < web.points.size(); splitIndex++) {
                std::vector<Web> candidateWebs = replaceWithSplit(webs, web.id, splitIndex, nextId);
                Graph<Web> candidateGraph = buildInterferenceGraph(candidateWebs);
                SplitCandidate candidate;
                candidate.valid = true;
                candidate.webId = web.id;
                candidate.splitIndex = splitIndex;
                candidate.resultingEdges = edgeCount(candidateGraph);
                candidate.resultingMaxDegree = maxDegree(candidateGraph);
                candidate.originalDegree = originalDegree;

                if (!best.valid ||
                    candidate.resultingEdges < best.resultingEdges ||
                    (candidate.resultingEdges == best.resultingEdges && candidate.resultingMaxDegree < best.resultingMaxDegree) ||
                    (candidate.resultingEdges == best.resultingEdges && candidate.resultingMaxDegree == best.resultingMaxDegree && candidate.originalDegree > best.originalDegree) ||
                    (candidate.resultingEdges == best.resultingEdges && candidate.resultingMaxDegree == best.resultingMaxDegree && candidate.originalDegree == best.originalDegree && candidate.webId < best.webId)) {
                    best = candidate;
                }
            }
        }
        return best;
    }

    ColoringResult splittingAllocation(const std::vector<Web>& originalWebs, int maxRegisters, int splitBudget) {
        ColoringResult basic = basicAllocation(originalWebs, maxRegisters);
        if (basic.success) return basic;

        std::vector<Web> currentWebs = originalWebs;
        for (int usedSplits = 1; usedSplits <= splitBudget; usedSplits++) {
            SplitCandidate candidate = chooseSplitCandidate(currentWebs);
            if (!candidate.valid) break;

            int newId = maxWebId(currentWebs) + 1;
            currentWebs = replaceWithSplit(currentWebs, candidate.webId, candidate.splitIndex, newId);

            ColoringResult attempt = basicAllocation(currentWebs, maxRegisters);
            if (attempt.success) return attempt;
        }
        return makeResult(false, originalWebs, {});
    }

    std::vector<int> graphIds(const Graph<Web>& graph) {
        std::vector<int> ids;
        for (auto vertex : graph.getVertexSet()) ids.push_back(vertex->getInfo().id);
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    std::unordered_map<int, std::set<int>> adjacencyById(const Graph<Web>& graph) {
        std::unordered_map<int, std::set<int>> adj;
        for (auto vertex : graph.getVertexSet()) {
            int id = vertex->getInfo().id;
            adj[id];
            for (auto edge : vertex->getAdj()) {
                adj[id].insert(edge->getDest()->getInfo().id);
            }
        }
        return adj;
    }

    bool dsaturBacktrack(const std::vector<int>& ids,
                         const std::unordered_map<int, std::set<int>>& adj,
                         int colorLimit,
                         std::unordered_map<int, int>& colors,
                         long long& calls,
                         long long callLimit) {
        if (++calls > callLimit) return false;
        if (colors.size() == ids.size()) return true;

        int chosen = -1;
        int bestSaturation = -1;
        int bestDegree = -1;
        for (int id : ids) {
            if (colors.count(id)) continue;
            std::set<int> neighborColors;
            auto itAdj = adj.find(id);
            if (itAdj != adj.end()) {
                for (int neighbor : itAdj->second) {
                    auto itColor = colors.find(neighbor);
                    if (itColor != colors.end()) neighborColors.insert(itColor->second);
                }
            }
            int saturation = static_cast<int>(neighborColors.size());
            int degree = itAdj == adj.end() ? 0 : static_cast<int>(itAdj->second.size());
            if (saturation > bestSaturation ||
                (saturation == bestSaturation && degree > bestDegree) ||
                (saturation == bestSaturation && degree == bestDegree && id < chosen)) {
                chosen = id;
                bestSaturation = saturation;
                bestDegree = degree;
            }
        }

        if (chosen == -1) return true;

        std::vector<bool> forbidden(colorLimit, false);
        auto itAdj = adj.find(chosen);
        if (itAdj != adj.end()) {
            for (int neighbor : itAdj->second) {
                auto itColor = colors.find(neighbor);
                if (itColor != colors.end() && itColor->second >= 0 && itColor->second < colorLimit) {
                    forbidden[itColor->second] = true;
                }
            }
        }

        for (int color = 0; color < colorLimit; color++) {
            if (forbidden[color]) continue;
            colors[chosen] = color;
            if (dsaturBacktrack(ids, adj, colorLimit, colors, calls, callLimit)) return true;
            colors.erase(chosen);
        }
        return false;
    }

    AllocationAttempt dsaturColor(const Graph<Web>& graph, int colorLimit) {
        AllocationAttempt attempt;
        if (colorLimit <= 0) return attempt;

        std::vector<int> ids = graphIds(graph);
        std::unordered_map<int, std::set<int>> adj = adjacencyById(graph);
        std::unordered_map<int, int> colors;
        long long calls = 0;
        const long long callLimit = 500000;

        if (!dsaturBacktrack(ids, adj, colorLimit, colors, calls, callLimit)) {
            return attempt;
        }

        attempt.success = true;
        attempt.colors = colors;
        for (const auto& [_, color] : colors) attempt.registersUsed = std::max(attempt.registersUsed, color + 1);
        return attempt;
    }

    AllocationAttempt tryDsaturUpTo(const Graph<Web>& graph, int maxRegisters) {
        for (int colors = 1; colors <= maxRegisters; colors++) {
            AllocationAttempt attempt = dsaturColor(graph, colors);
            if (attempt.success) return attempt;
        }
        return AllocationAttempt{};
    }

    ColoringResult freeAllocation(const std::vector<Web>& webs, int maxRegisters) {
        Graph<Web> graph = buildInterferenceGraph(webs);
        AllocationAttempt exact = tryDsaturUpTo(graph, maxRegisters);
        if (exact.success) return makeResult(true, webs, exact.colors);

        // Custom fallback: spill the fewest webs we can find, trying exact
        // combinations for small cases and degree-based greedy for larger ones.
        const int n = static_cast<int>(webs.size());
        std::vector<Web> ordered = spillCandidatesByHeuristic(graph);
        const long long combinationLimit = 30000;

        for (int spills = 1; spills <= n; spills++) {
            long long visited = 0;
            std::set<int> current;
            ColoringResult best;

            std::function<bool(int, int)> dfs = [&](int start, int left) -> bool {
                if (++visited > combinationLimit) return false;
                if (left == 0) {
                    std::vector<Web> kept = removeWebs(webs, current);
                    Graph<Web> reduced = buildInterferenceGraph(kept);
                    AllocationAttempt attempt = kept.empty() ? AllocationAttempt{true, {}, 0} : tryDsaturUpTo(reduced, maxRegisters);
                    if (!attempt.success) return false;
                    for (int id : current) attempt.colors[id] = -1;
                    best = makeResult(true, webs, attempt.colors);
                    return true;
                }
                for (int i = start; i <= n - left; i++) {
                    current.insert(ordered[i].id);
                    if (dfs(i + 1, left - 1)) return true;
                    current.erase(ordered[i].id);
                }
                return false;
            };

            if (dfs(0, spills) && best.success) return best;

            if (visited > combinationLimit) {
                std::set<int> greedy;
                for (int i = 0; i < spills; i++) greedy.insert(ordered[i].id);
                std::vector<Web> kept = removeWebs(webs, greedy);
                Graph<Web> reduced = buildInterferenceGraph(kept);
                AllocationAttempt attempt = kept.empty() ? AllocationAttempt{true, {}, 0} : tryDsaturUpTo(reduced, maxRegisters);
                if (attempt.success) {
                    for (int id : greedy) attempt.colors[id] = -1;
                    return makeResult(true, webs, attempt.colors);
                }
            }
        }

        return makeResult(false, webs, {});
    }
}

ColoringResult colorGraphFunc(Graph<Web> &graph, const int N, const std::string& mode, int K) {
    std::vector<Web> webs = graphWebs(graph);

    if (N <= 0) {
        std::cerr << "Assignment not feasible: number of registers must be positive.\n";
        return makeResult(false, webs, {});
    }

    if (mode == "basic") {
        ColoringResult result = basicAllocation(webs, N);
        if (!result.success) std::cerr << "Assignment not feasible\n";
        return result;
    }

    if (mode == "spilling") {
        ColoringResult result = spillingAllocation(webs, N, K);
        if (!result.success) std::cerr << "Assignment not feasible with spilling budget " << K << "\n";
        return result;
    }

    if (mode == "splitting") {
        ColoringResult result = splittingAllocation(webs, N, K);
        if (!result.success) std::cerr << "Assignment not feasible with splitting budget " << K << "\n";
        return result;
    }

    if (mode == "free") {
        ColoringResult result = freeAllocation(webs, N);
        if (!result.success) std::cerr << "Assignment not feasible in free mode\n";
        return result;
    }

    std::cerr << "Assignment not feasible: unknown algorithm '" << mode << "'.\n";
    return makeResult(false, webs, {});
}
