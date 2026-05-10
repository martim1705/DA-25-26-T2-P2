#ifndef PROJETO2_OUTPUT_H
#define PROJETO2_OUTPUT_H

#include <string>
#include <vector>
#include "Structures.h"

/**
 * @brief Writes webs and register assignments to the output file.
 * @complexity O(W log W + P), where W is the number of webs and P is the total
 * number of program points printed.
 */
void writeOutputToFile(const std::string& filename, const std::vector<Web>& webs, const ColoringResult& colorResult);

#endif //PROJETO2_OUTPUT_H
