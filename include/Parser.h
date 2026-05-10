#ifndef PROJETO2_PARSER_H
#define PROJETO2_PARSER_H

#include "Structures.h"

/**
 * @brief Reads the live-ranges file.
 * @complexity O(P), where P is the total number of program points in the file.
 */
std::vector<LiveRange> parseRangesFile(const std::string& filename);

/**
 * @brief Reads the register configuration file.
 * @complexity O(L), where L is the number of lines in the file.
 */
RegisterConfig parseRegistersFile(const std::string& filename);

#endif //PROJETO2_PARSER_H
