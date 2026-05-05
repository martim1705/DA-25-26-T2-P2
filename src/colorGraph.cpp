#include "colorGraph.h"
#include "Graph.h"
#include "Structures.h"
#include <stack>
#include <iostream>
#include <vector>

#include "InterferenceGraph.h"

//Hopcroft then NodeDegree for choosing vertex to spill or split
//Hopcroft-Tarjan finds articulation nodes, but it is not guaranteed to exist
//NodeDegree just blindly hopes the heaviest is the best to spill or split
//Features Optimistic Spilling resolution
//(stacking a web that is expected to go to memory into coloring stack, can be not set to memory after all, if the math is just right and the stars align)

// Heuristic Priority for Spilling/Splitting:
// 1. Hopcroft-Tarjan: Finds articulation nodes to break the graph apart (though not guaranteed to exist).
// 2. Max Node Degree: If no articulation node is found, blindly hopes the heaviest node is the best victim.
//
// * Features Optimistic Spilling Resolution *
// Stacks a web that is expected to go to memory into the coloring stack anyway.
// If the math works out and its neighbors share colors,
// it is rescued from memory and safely assigned a register after all.

namespace {
    //Hopcroft-Tarjan DFS Algorithm to find an articulation node if it exists
    //articulationDFS is a recursive helper and findArticulationNode is the "launcher"
    //O(V+E) Time Complexity, O(V) space complexity. <-this might be too much info...
    void articulationDFS(Vertex<Web>* u, std::unordered_map<int, bool>& visited,
                     std::unordered_map<int, int>& disc, std::unordered_map<int, int>& low,
                     std::unordered_map<int, int>& parent, std::unordered_map<int, bool>& is_ap,
                     int& time) {
        int children = 0;
        int u_id = u->getInfo().id;
        visited[u_id] = true;
        disc[u_id] = low[u_id] = ++time;

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

    bool findArticulationNode(const Graph<Web>& graph, Web& outVictim) {
        int time = 0;
        std::unordered_map<int, bool> visited;
        std::unordered_map<int, int> disc, low, parent;
        std::unordered_map<int, bool> is_ap;

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

    // Helper to find the Max Degree node cleanly
    Web findMaxDegreeNode(const Graph<Web>& graph) {
        int maxDegree = -1;
        Web victim;
        for (auto v : graph.getVertexSet()) {
            int currentDegree = v->getAdj().size();
            if (currentDegree > maxDegree) {
                maxDegree = currentDegree;
                victim = v->getInfo();
            }
        }
        return victim;
    }

    //Split using ProgramPoint vector
    bool splitWebAtLargestGap(const Web& victim, Web& partA, Web& partB, int newIdA, int newIdB) {
        if (victim.points.size() < 2) {
            return false; // Mathematically impossible to split
        }

        partA = victim;
        partB = victim;
        partA.id = newIdA;
        partB.id = newIdB;

        int maxGap = 0;
        size_t splitIndex = 0;

        // Find the largest gap in line numbers
        for(size_t i = 0; i < victim.points.size() - 1; i++) {
            int gap = victim.points[i+1].line - victim.points[i].line;
            if (gap > maxGap) {
                maxGap = gap;
                splitIndex = i;
            }
        }

        // Slice the vector of ProgramPoints
        partA.points = std::vector<ProgramPoint>(victim.points.begin(), victim.points.begin() + splitIndex + 1);
        partB.points = std::vector<ProgramPoint>(victim.points.begin() + splitIndex + 1, victim.points.end());
        return true;
    }

    ColoringResult assignColors(Graph<Web>& graph,std::stack<Web>& toColorStack,int N) {
        ColoringResult result;
        while (!toColorStack.empty()) {
            Web w = toColorStack.top();
            toColorStack.pop();
            std::vector<bool> usedColors(N, false);
            const Vertex<Web>* originalV = graph.findVertex(w);

            for (const auto edge : originalV->getAdj()) {
                int neighborId = edge->getDest()->getInfo().id;
                if (result.colorOfWeb.count(neighborId)) {
                    const int color = result.colorOfWeb[neighborId];
                    if (color>=0 && color < N) usedColors[color] = true;
                }
            }

            int colorToAssign = -1;
            for (int i = 0; i < N; i++) {
                if (!usedColors[i]) {
                    colorToAssign = i;
                    break;
                }
            }

            result.colorOfWeb[w.id] = colorToAssign;
        }
        result.success=true;
        return result;
    }
}




//Complexidade O(V^2) se for basic
//Ainda n vi a complexidade se n for... mas acho que O(V²) continua ser o pior que "trunfa" os outros.
ColoringResult colorGraphFunc(Graph<Web> &graph, const int N, const std::string& mode, int K) {

    //Cria uma cópia temporária
    Graph<Web> temp_graph;
    for (auto v : graph.getVertexSet()) temp_graph.addVertex(v->getInfo());
    for (auto v : graph.getVertexSet()) {
        for (const auto edge : v->getAdj()) {
            temp_graph.addEdge(v->getInfo(), edge->getDest()->getInfo(), 0);
        }
    }


    ColoringResult result;
    int budgetUsed = 0;
    std::stack<Web> toColorStack;
    int nextAvailableId = 9000;

    //Implementação do pseudo-código definido no pdf do projecto
    while (temp_graph.getNumVertex() > 0) {
        bool changed=false;
        Web nodeToRemove;

        for (auto v : temp_graph.getVertexSet()) {
            if (v->getAdj().size()<N) {
                nodeToRemove=(v->getInfo());
                changed=true;
                break;
            }
        }

        if(changed) {
            temp_graph.removeVertex(nodeToRemove);
            toColorStack.push(nodeToRemove);
        }
        else {
            //se não foi changed e o nr de vertices ainda é maior que zero,
            //é porque não conseguimos separar as webs para as colorir
            //entrar em modo de contingências...
            if (mode == " basic") {
                std::cerr<<"Assignment not feasible\n";
                result.success=false;
                return result;
            }
            if (K>0 && budgetUsed >= K) {
                std::cerr << "Assignment not feasible: Exceeded budget of " << K << " modifications.\n";
                result.success = false;
                return result;
            }
            budgetUsed++;
            //this only runs if it is not and changed and not basic
            //Find articulation node/Web
            //if no articulation found settle for the highest degree node. N garante nada...
            Web victim;
            if (!findArticulationNode(temp_graph, victim)) {
                victim = findMaxDegreeNode(temp_graph);
            }


            if (mode == " spilling") {// "dump" victim into memory,
                temp_graph.removeVertex(victim);
                toColorStack.push(victim);

            }
            else if (mode == " splitting") {//Find articulation node (Web) and split it creating two nodes
                //saving old edges before splitting
                std::vector<Web> oldNeighbors;
                for (auto edge : graph.findVertex(victim)->getAdj()) {
                    oldNeighbors.push_back(edge->getDest()->getInfo());
                }

                //Remove old vertex/Web
                temp_graph.removeVertex(victim);
                graph.removeVertex(victim);
                //Declare new ones
                Web partA, partB;

                int idA = nextAvailableId++;
                int idB = nextAvailableId++;

                //split
                if (!splitWebAtLargestGap(victim, partA, partB ,idA, idB)) {
                    std::cerr << "Assignment not feasible: Victim web " << victim.id << " cannot be split further.\n";
                    result.success = false;
                    return result;
                }
                //Add new ones to both graphs (temp and original for coloring)
                temp_graph.addVertex(partA);
                temp_graph.addVertex(partB);
                graph.addVertex(partA);
                graph.addVertex(partB);

                //Rebuild Edges using InterferenceGraph.h
                for (const auto& neighbor : oldNeighbors) {
                    if (interfere(partA, neighbor)) {
                        graph.addBidirectionalEdge(partA, neighbor, 0);
                        if (temp_graph.findVertex(neighbor) != nullptr)
                            temp_graph.addBidirectionalEdge(partA, neighbor, 0);
                    }
                    if (interfere(partB, neighbor)) {
                        graph.addBidirectionalEdge(partB, neighbor, 0);

                        if (temp_graph.findVertex(neighbor) != nullptr) {
                            temp_graph.addBidirectionalEdge(partB, neighbor, 0);
                        }
                    }
                }

            }
        }

    }
    return assignColors(graph, toColorStack, N);
}

