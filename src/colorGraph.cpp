#include "colorGraph.h"
#include "Graph.h"
#include "Structures.h"
#include <stack>
#include <iostream>
#include <vector>
namespace {
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
                    if (color < N) usedColors[color] = true;
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




//Complexidade O(V^2)
ColoringResult basicColorGraph(Graph<Web> &graph, const int N) {
    ColoringResult result;

    Graph<Web> temp_graph;
    for (auto v : graph.getVertexSet()) temp_graph.addVertex(v->getInfo());
    for (auto v : graph.getVertexSet()) {
        for (const auto edge : v->getAdj()) {
            temp_graph.addEdge(v->getInfo(), edge->getDest()->getInfo(), 0);
        }
    }



    std::stack<Web> toColorStack;
    bool changed=false;

    while (temp_graph.getNumVertex() > 0) {
        changed=false;
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
            std::cerr<<"Assignment not feasible\n";
            result.success=false;
            return result;
        }

    }
    return assignColors(graph, toColorStack, N);
}

