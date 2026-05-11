/**
 * @file colorGraph.cpp
 * @brief Register allocation algorithms based on graph coloring, spilling and splitting.
 */

#include "colorGraph.h"
#include "InterferenceGraph.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    /**
     * @brief Internal result of a single coloring attempt.
     */
    struct AllocationAttempt {
        bool success = false;
        std::unordered_map<int, int> colors;
        int registersUsed = 0;
    };

    /**
     * @brief Candidate split and its predicted graph-quality metrics.
     */
    struct SplitCandidate {
        bool valid = false;
        int webId = -1;
        size_t splitIndex = 0;
        int resultingEdges = 0;
        int resultingMaxDegree = 0;
        int originalDegree = 0;
    };

    /**
     * @brief Extracts webs from a graph in deterministic id order.
     * @param graph Interference graph.
     * @return Sorted vector of vertex webs.
     * @complexity O(V log V), where V is the number of vertices.
     */
    std::vector<Web> graphWebs(const Graph<Web>& graph) {
        std::vector<Web> webs;
        for (auto vertex : graph.getVertexSet()) {
            webs.push_back(vertex->getInfo());
        }
        std::sort(webs.begin(), webs.end(), [](const Web& a, const Web& b) { return a.id < b.id; });
        return webs;
    }

    /**
     * @brief Finds the largest web identifier in a vector.
     * @param webs Web collection.
     * @return Maximum id, or -1 for an empty vector.
     * @complexity O(V).
     */
    int maxWebId(const std::vector<Web>& webs) {
        int maxId = -1;
        for (const auto& web : webs) maxId = std::max(maxId, web.id);
        return maxId;
    }

    /**
     * @brief Counts undirected interference edges in a bidirectional graph.
     * @param graph Interference graph.
     * @return Number of undirected edges.
     * @complexity O(V + E), counting directed adjacency entries.
     */
    int edgeCount(const Graph<Web>& graph) {
        int directed = 0;
        for (auto vertex : graph.getVertexSet()) {
            directed += static_cast<int>(vertex->getAdj().size());
        }
        return directed / 2;
    }

    /**
     * @brief Computes the maximum degree of the graph.
     * @param graph Interference graph.
     * @return Maximum adjacency-list size among vertices.
     * @complexity O(V).
     */
    int maxDegree(const Graph<Web>& graph) {
        int best = 0;
        for (auto vertex : graph.getVertexSet()) {
            best = std::max(best, static_cast<int>(vertex->getAdj().size()));
        }
        return best;
    }

    /**
     * @brief Validates that no adjacent non-spilled webs share the same color.
     * @param graph Interference graph.
     * @param colors Web-id to register mapping.
     * @return true if a conflict exists.
     * @complexity O(V + E) average-case, assuming unordered_map lookup is O(1).
     */
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

    /**
     * @brief Tries to color a graph with a fixed number of colors using simplification and optimistic coloring.
     *
     * Vertices with degree below colorCount are removed first.  When none exists, the current
     * highest-degree vertex is pushed optimistically and the final coloring phase determines whether
     * that choice was safe.
     *
     * @param originalGraph Graph to color.
     * @param colorCount Number of colors/registers to try.
     * @return AllocationAttempt with success=false if no valid assignment is found.
     * @complexity O(V^2 + E) for one fixed color count, dominated by repeated vertex scans and removals.
     */
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
                // push the highest-degree node anyway.
                int bestDegree = -1;
                for (const auto vertex : tempGraph.getVertexSet()) {
                    int degree = static_cast<int>(vertex->getAdj().size());
                    if (best == nullptr || degree > bestDegree ||
                       (degree == bestDegree && vertex->getInfo().id < best->getInfo().id)) {
                        best = vertex;
                        bestDegree = degree;
                       }
                }
            }

           if (best == nullptr) break;

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

    /**
     * @brief Runs simplification coloring from one register up to the register limit.
     * @param graph Graph to color.
     * @param maxRegisters Maximum number of colors allowed.
     * @return First successful attempt, or failure if none succeeds.
     * @complexity O(C * (V^2 + E)), where C is maxRegisters.
     */
    AllocationAttempt tryBasicUpTo(const Graph<Web>& graph, int maxRegisters) {
        for (int colors = 1; colors <= maxRegisters; colors++) {
            AllocationAttempt attempt = simplifyAndColor(graph, colors);
            if (attempt.success) return attempt;
        }
        return AllocationAttempt{};
    }

    /**
     * @brief Packs a public coloring result from internal data.
     * @param success Whether allocation succeeded.
     * @param webs Webs to expose as the final web set.
     * @param colors Web-id to register mapping.
     * @return ColoringResult ready for output.
     * @complexity O(V + A), copying webs and assignments.
     */
    ColoringResult makeResult(bool success, const std::vector<Web>& webs, const std::unordered_map<int, int>& colors) {
        ColoringResult result;
        result.success = success;
        result.finalWebs = webs;
        result.colorOfWeb = colors;
        return result;
    }

    /**
     * @brief Performs basic register allocation by building the graph and coloring it.
     * @param webs Webs to allocate.
     * @param maxRegisters Maximum registers available.
     * @return Successful coloring result or an infeasible result.
     * @complexity O(V^2 * P^2 + C * (V^2 + E)).
     */
    ColoringResult basicAllocation(const std::vector<Web>& webs, int maxRegisters) {
        Graph<Web> graph = buildInterferenceGraph(webs);
        AllocationAttempt attempt = tryBasicUpTo(graph, maxRegisters);
        if (!attempt.success) return makeResult(false, webs, {});
        return makeResult(true, webs, attempt.colors);
    }

    /**
     * @brief Creates a vector without the selected web ids.
     * @param webs Original web vector.
     * @param idsToRemove Web ids to omit.
     * @return Filtered web vector.
     * @complexity O(V log R), where R is the number of ids to remove.
     */
    std::vector<Web> removeWebs(const std::vector<Web>& webs, const std::set<int>& idsToRemove) {
        std::vector<Web> kept;
        for (const auto& web : webs) {
            if (!idsToRemove.count(web.id)) kept.push_back(web);
        }
        return kept;
    }

    /**
     * @brief Helper to find the highest degree node cleanly.
     * @complexity O(V)
     */
    Web findMaxDegreeNode(const Graph<Web>& graph) {
        int maxDegree = -1;
        Web victim;
        for (auto v : graph.getVertexSet()) {
            int currentDegree = static_cast<int>(v->getAdj().size());
            if (currentDegree > maxDegree ||
               (currentDegree == maxDegree && v->getInfo().id < victim.id)) {
                maxDegree = currentDegree;
                victim = v->getInfo();
            }
        }
        return victim;
    }

    /**
     * @brief Hopcroft-Tarjan DFS algorithm to find an articulation point.
     * @complexity O(V+E)
     */
    void articulationDFS(const Vertex<Web>* u, std::unordered_map<int, bool>& visited,
                         std::unordered_map<int, int>& disc, std::unordered_map<int, int>& low,
                         std::unordered_map<int, int>& parent, std::unordered_map<int, bool>& is_ap,
                         int& time) {
        int u_id = u->getInfo().id;
        visited[u_id] = true;
        disc[u_id] = low[u_id] = ++time;
        int children = 0;

        for (auto edge : u->getAdj()) {
            Vertex<Web>* v = edge->getDest();
            int v_id = v->getInfo().id;

            if (!visited[v_id]) {
                children++;
                parent[v_id] = u_id;
                articulationDFS(v, visited, disc, low, parent, is_ap, time);
                low[u_id] = std::min(low[u_id], low[v_id]);

                if (parent[u_id] == -1 && children > 1) is_ap[u_id] = true;
                if (parent[u_id] != -1 && low[v_id] >= disc[u_id]) is_ap[u_id] = true;
            } else if (v_id != parent[u_id]) {
                low[u_id] = std::min(low[u_id], disc[v_id]);
            }
        }
    }

    /**
     * @brief Finds an articulation node if it exists.
     * @complexity O(V+E)
     */
    bool findArticulationNode(const Graph<Web>& graph, Web& outVictim) {
        std::unordered_map<int, bool> visited;
        std::unordered_map<int, int> disc, low, parent;
        std::unordered_map<int, bool> is_ap; // FIXED: Separated from the ints
        int time = 0;

        for (auto v : graph.getVertexSet()) {
            int id = v->getInfo().id;
            visited[id] = false;
            parent[id] = -1;
            is_ap[id] = false;
        }

        for (auto v : graph.getVertexSet()) {
            if (!visited[v->getInfo().id]) {
                articulationDFS(v, visited, disc, low, parent, is_ap, time);
            }
        }

        for (auto v : graph.getVertexSet()) {
            if (is_ap[v->getInfo().id]) {
                outVictim = v->getInfo();
                return true;
            }
        }
        return false;
    }


    /**
     * @brief Performs allocation with bounded web spilling using Hopcroft-Tarjan.
     * * Instead of exponential brute-force combinations, this safely breaks the
     * interference graph at structural weak points (articulation nodes) in polynomial time.
     * * @param webs Webs to allocate.
     * @param maxRegisters Maximum registers available.
     * @param spillBudget Maximum number of webs that may be moved to memory.
     * @return Coloring result with spilled webs marked as memory, or failure.
     * @complexity O(K * (V^2 + E)), where K is the spill budget.
     */
    ColoringResult spillingAllocation(const std::vector<Web>& webs, int maxRegisters, int spillBudget) {
        ColoringResult basic = basicAllocation(webs, maxRegisters);
        if (basic.success) return basic;

        std::vector<Web> currentWebs = webs;
        std::set<int> spilledIds;
        const int maxSpills = std::min(spillBudget, static_cast<int>(webs.size()));

        for (int spills = 1; spills <= maxSpills; spills++) {
            Graph<Web> graph = buildInterferenceGraph(currentWebs);
            Web victim;

            // Heuristic 1: Shatter the graph at an articulation point
            if (!findArticulationNode(graph, victim)) {
                // Heuristic 2: Fallback to the most entangled node
                victim = findMaxDegreeNode(graph);
            }

            spilledIds.insert(victim.id);
            currentWebs = removeWebs(currentWebs, {victim.id});

            Graph<Web> reducedGraph = buildInterferenceGraph(currentWebs);
            AllocationAttempt attempt = tryBasicUpTo(reducedGraph, maxRegisters);

            if (attempt.success) {
                for (int id : spilledIds) attempt.colors[id] = -1;
                return makeResult(true, webs, attempt.colors);
            }
        }

        return makeResult(false, webs, {});
    }

    /**
     * @brief Sorts a web's program points by line number.
     * @param web Web to sort in place.
     * @complexity O(Q log Q), where Q is the number of points in the web.
     */
    void sortWebPoints(Web& web) {
        std::sort(web.points.begin(), web.points.end(), [](const ProgramPoint& a, const ProgramPoint& b) {
            return a.line < b.line;
        });
    }

    /**
     * @brief Splits a web into two consecutive derived webs.
     * @param original Web to split.
     * @param splitIndex Last point index kept in the left web.
     * @param newId Identifier assigned to the right derived web.
     * @param left Output left derived web.
     * @param right Output right derived web.
     * @return true if the split is valid.
     * @complexity O(Q), where Q is the number of points copied from the original web.
     */
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

    /**
     * @brief Replaces one web in a collection with the two webs produced by a split.
     * @param webs Current web collection.
     * @param webId Identifier of the web to split.
     * @param splitIndex Split boundary passed to splitWeb().
     * @param newId Identifier assigned to the new right-hand web.
     * @return Updated web collection sorted by id.
     * @complexity O(V log V + Q), where Q is the point count of the split web.
     */
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

    /**
     * @brief Chooses the best heuristic split for the current web set.
     *
     * Each possible split is tested by rebuilding the interference graph.  Candidates are ranked by
     * resulting edge count, resulting maximum degree, original degree and web id.
     *
     * @param webs Current web set.
     * @return Best candidate, or an invalid candidate if no split can be made.
     * @complexity O(S * (V^2 * P^2 + V^2)), where S is the number of tested split positions.
     */
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

    /**
     * @brief Performs allocation with bounded heuristic web splitting.
     *
     * Basic allocation is attempted first.  If it fails, the algorithm repeatedly selects the split
     * that most reduces interference according to edge count and maximum degree, then rebuilds the graph.
     *
     * @param originalWebs Webs to allocate.
     * @param maxRegisters Maximum registers available.
     * @param splitBudget Maximum number of webs to split.
     * @return Coloring result over the possibly split web set, or failure.
     * @complexity O(K * S * (V^2 * P^2) + K * C * (V^2 + E)), where K is splitBudget.
     */
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

    /**
     * @brief Extracts web identifiers from a graph in sorted order.
     * @param graph Interference graph.
     * @return Sorted vector of web ids.
     * @complexity O(V log V).
     */
    std::vector<int> graphIds(const Graph<Web>& graph) {
        std::vector<int> ids;
        for (auto vertex : graph.getVertexSet()) ids.push_back(vertex->getInfo().id);
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    /**
     * @brief Converts the graph adjacency lists to an id-based adjacency map.
     * @param graph Interference graph.
     * @return Mapping from web id to sorted neighboring web ids.
     * @complexity O(V + E log D), where D is the maximum degree.
     */
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

    /**
     * @brief Recursive DSATUR coloring search with a call limit.
     *
     * The next vertex is chosen by maximum saturation degree, then ordinary degree, then id.
     * Backtracking tries every still-available color until a complete assignment is found.
     *
     * @param ids Sorted web ids still considered by the graph.
     * @param adj Id-based adjacency map.
     * @param colorLimit Number of colors/registers allowed.
     * @param colors Partial assignment updated in place.
     * @param calls Number of recursive calls already performed.
     * @param callLimit Hard cap on recursive calls.
     * @return true if a full coloring was found before the cap.
     * @complexity Exponential in V in the worst case, with each call scanning vertices and neighbors.
     */
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

    /**
     * @brief Attempts exact DSATUR coloring with a fixed color count.
     * @param graph Interference graph.
     * @param colorLimit Number of colors/registers to try.
     * @return Successful allocation attempt, or failure if no coloring is found before the cap.
     * @complexity Exponential in V in the worst case, bounded in practice by the call limit.
     */
    AllocationAttempt dsaturColor(const Graph<Web>& graph, int colorLimit) {
        AllocationAttempt attempt;
        if (colorLimit <= 0) return attempt;

        std::vector<int> ids = graphIds(graph);
        std::unordered_map<int, std::set<int>> adj = adjacencyById(graph);
        std::unordered_map<int, int> colors;
        long long calls = 0;
        constexpr long long callLimit = 500000;

        if (!dsaturBacktrack(ids, adj, colorLimit, colors, calls, callLimit)) {
            return attempt;
        }

        attempt.success = true;
        attempt.colors = colors;
        for (const auto& [_, color] : colors) attempt.registersUsed = std::max(attempt.registersUsed, color + 1);
        return attempt;
    }

    /**
     * @brief Runs DSATUR from one register up to the register limit.
     * @param graph Graph to color.
     * @param maxRegisters Maximum colors available.
     * @return First exact DSATUR coloring found, or failure.
     * @complexity Exponential in V in the worst case, repeated for at most C register counts.
     */
    AllocationAttempt tryDsaturUpTo(const Graph<Web>& graph, int maxRegisters) {
        for (int colors = 1; colors <= maxRegisters; colors++) {
            AllocationAttempt attempt = dsaturColor(graph, colors);
            if (attempt.success) return attempt;
        }
        return AllocationAttempt{};
    }

    /**
         * @brief Custom allocation strategy based on DSATUR with Hopcroft-Tarjan spilling fallback.
         * @param webs Webs to allocate.
         * @param maxRegisters Maximum registers available.
         * @return Exact coloring if possible; otherwise a coloring with heuristically spilled webs.
         */
    ColoringResult freeAllocation(const std::vector<Web>& webs, int maxRegisters) {
        Graph<Web> graph = buildInterferenceGraph(webs);

        // 1. Try an exact mathematical coloring first
        AllocationAttempt exact = tryDsaturUpTo(graph, maxRegisters);
        if (exact.success) return makeResult(true, webs, exact.colors);

        // 2. If it fails, use polynomial heuristics to spill.
        std::vector<Web> currentWebs = webs;
        std::set<int> spilledIds;
        const int n = static_cast<int>(webs.size());

        for (int spills = 1; spills <= n; spills++) {
            Graph<Web> currentGraph = buildInterferenceGraph(currentWebs);
            Web victim;

            // Heuristic 1: Break at articulation node
            if (!findArticulationNode(currentGraph, victim)) {
                // Heuristic 2: Fallback to max degree
                victim = findMaxDegreeNode(currentGraph);
            }

            spilledIds.insert(victim.id);
            currentWebs = removeWebs(currentWebs, {victim.id});

            // Re-test the newly weakened graph with the exact DSATUR algorithm
            Graph<Web> reducedGraph = buildInterferenceGraph(currentWebs);
            AllocationAttempt attempt = tryDsaturUpTo(reducedGraph, maxRegisters);

            if (attempt.success) {
                for (int id : spilledIds) attempt.colors[id] = -1;
                return makeResult(true, webs, attempt.colors);
            }
        }

        return makeResult(false, webs, {});
    }
}

ColoringResult colorGraphFunc(const Graph<Web> &graph, const int N, const std::string& mode, int K) {
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
