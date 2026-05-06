#ifndef PROJETO2_OUTPUT_H
#define PROJETO2_OUTPUT_H

#include <string>
#include <vector>
#include "Structures.h"

// Writes the webs and the register allocation result to the specified file
void writeOutputToFile(const std::string& filename, const std::vector<Web>& webs, const ColoringResult& colorResult);

#endif //PROJETO2_OUTPUT_H