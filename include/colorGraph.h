#ifndef PROJETO2_COLORGRAPH_H
#define PROJETO2_COLORGRAPH_H
#include "Graph.h"
#include "Structures.h"

/**
 * @brief Allocates registers according to the selected algorithm.
 *
 * Supported modes:
 * - basic: simplification-stack graph coloring only;
 * - spilling: basic first, then removes up to K webs to memory;
 * - splitting: basic first, then splits up to K webs;
 * - free: DSATUR-based custom allocator with automatic spilling fallback.
 *
 * @complexity The basic allocator is O(C * (V^2 + E)) for C tested color
 * counts up to the register limit.  Spilling may enumerate combinations and is
 * exponential in the spill budget K in the exact small-case path.  Splitting is
 * heuristic and rebuilds the graph after each split.  The free DSATUR allocator
 * is exponential in the worst case, as expected for graph coloring, with a
 * greedy fallback for large spill searches.
 */
ColoringResult colorGraphFunc(Graph<Web> &graph, int N, const std::string& mode, int K);

#endif //PROJETO2_COLORGRAPH_H
