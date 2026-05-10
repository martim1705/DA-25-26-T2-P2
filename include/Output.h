/**
 * @file Output.h
 * @brief Output writer for web listings and register assignments.
 */

#ifndef PROJETO2_OUTPUT_H
#define PROJETO2_OUTPUT_H

#include <string>
#include <vector>
#include "Structures.h"

/**
 * @brief Writes webs and register assignments to the output file required by the statement.
 *
 * If allocation succeeds, each web is printed with either `rX` for its register or `M` for memory.
 * If allocation fails, the number of registers is written as zero and every web is assigned to memory.
 *
 * @param filename Path to the output allocation file.
 * @param webs Original webs produced before allocation.
 * @param colorResult Result returned by the selected coloring algorithm.
 * @complexity O(W log W + P + A), where W is the number of webs, P is the total number
 * of program points printed and A is the number of assignments in the coloring map.
 */
void writeOutputToFile(const std::string& filename, const std::vector<Web>& webs, const ColoringResult& colorResult);

#endif //PROJETO2_OUTPUT_H
