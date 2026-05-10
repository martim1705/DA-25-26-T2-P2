#ifndef PROJETO2_WEB_H
#define PROJETO2_WEB_H
#include "Structures.h"

/**
 * @brief Builds live webs by merging live ranges of the same variable that share
 * at least one program point.
 * @complexity O(R^2 * P^2) in the worst case per variable, where R is the
 * number of ranges of the variable and P is the maximum number of points in a
 * range.  The final sorting step costs O(W * Q log Q), where Q is the number of
 * points in one web.
 */
std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges);

/** @brief Returns true when two live ranges share a program line. */
bool intersects(const LiveRange& a, const LiveRange& b);

/** @brief Returns true when two program points refer to the same line. */
bool sameLine(const ProgramPoint& a, const ProgramPoint& b);

/** @brief Returns true when a web shares a program line with a live range. */
bool intersects(const Web& web, const LiveRange& range);
#endif //PROJETO2_WEB_H
