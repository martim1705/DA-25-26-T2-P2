#ifndef PROJETO2_WEB_H
#define PROJETO2_WEB_H
#include "Structures.h"

std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges);

bool intersects(const LiveRange& a, const LiveRange& b);

bool sameLine(const ProgramPoint& a, const ProgramPoint& b);

bool intersects(const Web& web, const LiveRange& range);
#endif //PROJETO2_WEB_H
