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

    for ( const auto& range : ranges) {
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
            webs.push_back(web);
        }
    }

    return webs;
}


bool intersects(const Web& web, const LiveRange& range) {
    return true;
}