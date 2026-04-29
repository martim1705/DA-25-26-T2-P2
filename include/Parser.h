#ifndef PROJETO2_PARSER_H
#define PROJETO2_PARSER_H

#include "Structures.h"

std::vector<LiveRange> parseRangesFile(const std::string& filename);

RegisterConfig parseRegistersFile(const std::string& filename);


#endif //PROJETO2_PARSER_H
