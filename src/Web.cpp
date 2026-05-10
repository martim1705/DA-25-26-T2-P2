#include "Web.h"
#include <vector>
#include <map>
#include <algorithm>

namespace {
    void normalizeWebPoints(Web& web) {
        std::sort(web.points.begin(), web.points.end(), [](const ProgramPoint& a, const ProgramPoint& b) {
            if (a.line != b.line) return a.line < b.line;
            if (a.isStart != b.isStart) return a.isStart > b.isStart;
            return a.isEnd > b.isEnd;
        });

        std::vector<ProgramPoint> cleanPoints;
        for (const auto& p : web.points) {
            if (cleanPoints.empty() || cleanPoints.back().line != p.line) {
                cleanPoints.push_back(p);
            } else {
                cleanPoints.back().isStart = cleanPoints.back().isStart || p.isStart;
                cleanPoints.back().isEnd = cleanPoints.back().isEnd || p.isEnd;
            }
        }

        // When a live range of the same variable ends and another begins at the
        // same instruction, the assignment requires us to fuse the ranges and
        // treat that point as a normal interior point of the resulting web.
        for (auto& p : cleanPoints) {
            if (p.isStart && p.isEnd) {
                p.isStart = false;
                p.isEnd = false;
            }
        }
        web.points = cleanPoints;
    }
}

std::vector<Web> buildWebs(const std::vector<LiveRange>& ranges) {
    std::map<std::string, std::vector<LiveRange>> groupedRanges;
    for (const auto& range : ranges) {
        groupedRanges[range.variable].push_back(range);
    }

    std::vector<Web> webs;
    int nextWebId = 0;

    for (auto& [variable, varRanges] : groupedRanges) {
        std::vector<bool> used(varRanges.size(), false);

        for (size_t i = 0; i < varRanges.size(); i++) {
            if (used[i]) continue;

            Web web;
            web.id = nextWebId++;
            web.variable = variable;
            web.points = varRanges[i].points;
            used[i] = true;

            bool changed = true;
            while (changed) {
                changed = false;
                for (size_t j = 0; j < varRanges.size(); j++) {
                    if (used[j]) continue;
                    if (intersects(web, varRanges[j])) {
                        web.points.insert(web.points.end(), varRanges[j].points.begin(), varRanges[j].points.end());
                        used[j] = true;
                        changed = true;
                    }
                }
            }

            normalizeWebPoints(web);
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
        for (const ProgramPoint& p2 : b.points) {
            if (sameLine(p1, p2)) return true;
        }
    }
    return false;
}

bool intersects(const Web& web, const LiveRange& range) {
    for (const ProgramPoint& p1 : web.points) {
        for (const ProgramPoint& p2 : range.points) {
            if (sameLine(p1, p2)) return true;
        }
    }
    return false;
}
