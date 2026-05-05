#ifndef PROJETO2_COLORGRAPH_H
#define PROJETO2_COLORGRAPH_H
#include "Graph.h"
#include "Structures.h"

//Complexidade O(V^2) em basic mode (ainda n vi os outros, acho provavel que seja na mesma O(V²)
ColoringResult colorGraphFunc(Graph<Web> &graph, int N, const std::string& mode, int K);



#endif //PROJETO2_COLORGRAPH_H
