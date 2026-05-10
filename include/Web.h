/**
 * @file Web.h
 * @brief Live-web construction from raw live ranges.
 */

#ifndef PROJETO2_WEB_H
#define PROJETO2_WEB_H
#include "Structures.h"

/**
 * @brief Builds live webs by merging live ranges of the same variable that share at least one program point.
 *
 * Ranges are first grouped by variable.  For each variable, a greedy closure is performed: once a range
 * is inserted into the current web, every still-unused range that intersects that growing web is also merged.
 * Points are then sorted and duplicate lines are normalized.
 *
 * @param ranges Raw live ranges parsed from the input file.
 * @return Vector of normalized webs with consecutive ids.
 * @complexity O(R^2 * P^2) in the worst case per variable, where R is the number of ranges
 * of the variable and P is the maximum number of points in a range.  The final sorting step costs
 * O(W * Q log Q), where W is the number of webs and Q is the maximum number of points in one web.
 */
std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges);

/**
 * @brief Returns true when two live ranges share at least one program line.
 * @param a First live range.
 * @param b Second live range.
 * @return true if any program line appears in both ranges.
 * @complexity O(Pa * Pb), where Pa and Pb are the number of points in each range.
 */
bool intersects(const LiveRange& a, const LiveRange& b);

/**
 * @brief Returns true when two program points refer to the same source line.
 * @param a First point.
 * @param b Second point.
 * @return true if both points have the same line number.
 * @complexity O(1).
 */
bool sameLine(const ProgramPoint& a, const ProgramPoint& b);

/**
 * @brief Returns true when a web shares at least one program line with a live range.
 * @param web Web to test.
 * @param range Live range to test.
 * @return true if any program line appears in both collections.
 * @complexity O(Pw * Pr), where Pw and Pr are the number of points in the web and range.
 */
bool intersects(const Web& web, const LiveRange& range);
#endif //PROJETO2_WEB_H
