// build web
// For each variable:
//      Joins ranges that intersect each other
//      Creates a web for each group
#include "Web.h"
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>

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

            //Tratamento do vector. (Para o output ser correcto, logicamente já funcionava isto).
            // 1. Ordenar os pontos por ordem crescente
            std::sort(web.points.begin(), web.points.end(), [](const ProgramPoint& a, const ProgramPoint& b) {
                return a.line < b.line;
            });

            //remove duplicados adicionando apenas nrs diferentes, e se já existirem apenas adiciona start e end flags
            std::vector<ProgramPoint> cleanPoints;
            for (const auto& p : web.points) {
                if (cleanPoints.empty() || cleanPoints.back().line != p.line) {
                    // new number
                    cleanPoints.push_back(p);
                } else {
                    // repeat!
                    cleanPoints.back().isStart = cleanPoints.back().isStart || p.isStart;
                    cleanPoints.back().isEnd = cleanPoints.back().isEnd || p.isEnd;
                }
            }

            // End e Start n podem coexistir
            // Isto está num loop extra final para ter a certeza que novas branchs, com um mesmo nr, n provoquem inclusao de sinal a remover.
            for (auto& p : cleanPoints) {
                if (p.isStart && p.isEnd) {
                    p.isStart = false;
                    p.isEnd = false;
                }
            }

            //Replace it
            web.points = cleanPoints;


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