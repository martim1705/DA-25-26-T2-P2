// build web
// For each variable:
//      Joins ranges that intersect each other
//      Creates a web for each group
#include "Web.h"
#include <vector>
#include <iostream>
#include <unordered_map>

std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges) {
    std::unordered_map<std::string, std::vector<LiveRange>> group;

    for (const auto& range : ranges) {
        group[range.variable].push_back(range);
    }

    std::vector<Web> webs;
    int nextWebId = 0;

    for (auto& [var, varRanges] : group) {
        std::vector<bool> used(varRanges.size(), false);

        for (size_t i = 0; i < varRanges.size(); i++) {
            if (used[i]) continue;

            Web web;
            web.id = nextWebId++;
            web.variable = var;
            web.points = varRanges[i].points;

            used[i] = true;

            bool changed = true;

            while (changed) {
                changed = false;

                for (size_t j = 0; j < varRanges.size(); j++) {
                    if (used[j]) continue;

                    if (intersects(web, varRanges[j])) {
                        for (const ProgramPoint& p : varRanges[j].points) {
                            web.points.push_back(p);
                        }

                        used[j] = true;
                        changed = true;
                    }
                }
            }

            webs.push_back(web);
        }
    }

    return webs;
}

bool sameLine(const ProgramPoint& a, const ProgramPoint& b) {
    return a.line == b.line;
}

bool intersects(const LiveRange& a, const LiveRange& b) {
    for (const ProgramPoint& p1 : a.points) {
        for (const ProgramPoint &p2: b.points) {
            if (sameLine(p1, p2)) {
                return true;
            }

        }
    }
    return false;
}

bool intersects(const Web& web, const LiveRange& range) {
    for (const ProgramPoint& p1 : web.points) {
        for (const ProgramPoint &p2: range.points) {
            if (sameLine(p1, p2)) {
                return true;
            }

        }
    }
    return false;
}