/**
 * @file InterferenceGraph.h
 * @brief Construction and pairwise testing of live-web interference graphs.
 */

#ifndef PROJETO2_INTERFERENCEGRAPH_H
#define PROJETO2_INTERFERENCEGRAPH_H
#include "Graph.h"
#include "Structures.h"

/**
 * @brief Builds the interference graph whose vertices are webs and whose edges connect incompatible webs.
 *
 * Two webs are connected when they are alive at the same program point, except for the special
 * start/end boundary where one value is defined exactly when the other value is read for the last time.
 *
 * @param webs Webs to insert as graph vertices.
 * @return Graph<Web> containing one vertex per web and bidirectional edges for interference.
 * @complexity O(W^2 * P^2 + W^2), where W is the number of webs and P is the maximum
 * number of points in a web.  The W^2 term comes from vertex lookups while adding edges.
 */
Graph<Web> buildInterferenceGraph(const std::vector<Web>& webs);

/**
 * @brief Returns true if two webs interfere.
 *
 * A shared line normally means interference, except for the assignment-like boundary where
 * one web starts at the same instruction where the other ends.  That exception allows safe
 * register reuse for values whose live ranges merely touch at a definition/use boundary.
 *
 * @param webA First web.
 * @param webB Second web.
 * @return true if both webs are simultaneously live at any relevant program point.
 * @complexity O(Pa * Pb), where Pa and Pb are the numbers of points in each web.
 */
bool interfere(const Web& webA, const Web& webB);
#endif //PROJETO2_INTERFERENCEGRAPH_H
