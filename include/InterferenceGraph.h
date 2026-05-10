#ifndef PROJETO2_INTERFERENCEGRAPH_H
#define PROJETO2_INTERFERENCEGRAPH_H
#include "Graph.h"
#include "Structures.h"

/**
 * @brief Builds the interference graph whose vertices are webs and whose edges
 * connect webs that are alive at the same program point.
 * @complexity O(W^2 * P^2), where W is the number of webs and P is the maximum
 * number of points in a web.
 */
Graph<Web> buildInterferenceGraph(const std::vector<Web>& webs);

/**
 * @brief Returns true if two webs interfere.
 *
 * A shared line normally means interference, except for the assignment-like
 * boundary where one web starts at the same instruction where the other ends.
 * @complexity O(Pa * Pb), where Pa and Pb are the web sizes.
 */
bool interfere(const Web& webA, const Web& webB);
#endif //PROJETO2_INTERFERENCEGRAPH_H
