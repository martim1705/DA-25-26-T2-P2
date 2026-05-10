/**
 * @file Parser.h
 * @brief Parsers for live-range and register-configuration input files.
 */

#ifndef PROJETO2_PARSER_H
#define PROJETO2_PARSER_H

#include "Structures.h"

/**
 * @brief Reads and validates the live-ranges file.
 *
 * Blank lines and lines starting with `#` are ignored.  Each valid line is expected to have
 * the form `variable: p1,p2,...`, where a point may end in `+` or `-` to mark starts/ends.
 * Malformed points are reported to stderr and skipped so the remaining input can still be used.
 *
 * @param filename Path to the ranges file.
 * @return Vector of parsed live ranges.
 * @complexity O(L + P), where L is the total number of characters read and P is the number
 * of comma-separated program-point tokens.
 */
std::vector<LiveRange> parseRangesFile(const std::string& filename);

/**
 * @brief Reads the register configuration file.
 *
 * Recognizes `registers: N` and `algorithm: mode` or `algorithm: mode, K`.  Invalid numeric
 * values are reported to stderr and leave the corresponding field at its default value.
 *
 * @param filename Path to the register configuration file.
 * @return Parsed register allocation configuration.
 * @complexity O(L), where L is the number of characters in the file.
 */
RegisterConfig parseRegistersFile(const std::string& filename);

#endif //PROJETO2_PARSER_H
