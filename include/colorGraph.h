/**
 * @file colorGraph.h
 * @brief Public entry point for register allocation through graph coloring.
 */

#ifndef PROJETO2_COLORGRAPH_H
#define PROJETO2_COLORGRAPH_H
#include "Graph.h"
#include "Structures.h"

/**
 * @brief Allocates registers according to the selected algorithm.
 *
 * Supported modes:
 * - `basic`: simplification-stack graph coloring only;
 * - `spilling`: basic first, then removes up to K webs to memory;
 * - `splitting`: basic first, then splits up to K webs;
 * - `free`: DSATUR-based custom allocator with automatic spilling fallback.
 *
 * The function receives an already-built interference graph, extracts the webs, and
 * dispatches to the algorithm requested by the register configuration file.
 *
 * @param graph Interference graph whose vertices are live webs and whose edges are conflicts.
 * @param N Maximum number of physical registers available.
 * @param mode Allocation strategy name: `basic`, `spilling`, `splitting` or `free`.
 * @param K Optional strategy parameter used as the spill/split budget.
 * @return ColoringResult with the final webs and the register or memory assignment for each web.
 * @complexity Let V be the number of webs, E the number of interference edges, P the maximum
 * number of program points per web, C the register limit and K the spill/split budget.
 * Basic allocation costs O(C * (V^2 + E)) after graph construction.  Spilling may enumerate
 * O(sum_{i=1..K} binom(V,i)) spill sets, rebuilding the graph in O(V^2 * P^2) for each tested
 * set.  Splitting performs up to K heuristic graph rebuilds, each dominated by O(V^2 * P^2).
 * Free mode uses DSATUR, exponential in V in the worst case, then tries bounded spilling.
 */
ColoringResult colorGraphFunc(Graph<Web> &graph, int N, const std::string& mode, int K);

#endif //PROJETO2_COLORGRAPH_H
