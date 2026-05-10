/**
 * @file Output.cpp
 * @brief Implementation of the allocation output writer.
 */

#include "Output.h"
#include <fstream>
#include <iostream>
#include <algorithm>

void writeOutputToFile(const std::string& filename, const std::vector<Web>& webs, const ColoringResult& colorResult) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error opening output file: " << filename << "\n";
        return;
    }

    const std::vector<Web>& websToPrint = (!colorResult.finalWebs.empty()) ? colorResult.finalWebs : webs;

    outFile << "# Total number of webs followed by the listing of the program points of each one\n";
    outFile << "# program points in each web are sorted in ascending order\n";
    outFile << "webs: " << websToPrint.size() << "\n";

    std::vector<Web> sortedWebs = websToPrint;
    std::sort(sortedWebs.begin(), sortedWebs.end(), [](const Web& a, const Web& b) {
        return a.id < b.id;
    });

    for (const auto& w : sortedWebs) {
        outFile << "web" << w.id << ": ";
        for (size_t i = 0; i < w.points.size(); ++i) {
            const auto& p = w.points[i];
            outFile << p.line;
            if (p.isStart) outFile << "+";
            if (p.isEnd) outFile << "-";
            if (i + 1 < w.points.size()) outFile << ",";
        }
        outFile << "\n";
    }

    outFile << "# Total number of registers used, followed by assignment to webs\n";

    if (colorResult.success) {
        int maxColorUsed = -1;
        for (const auto& [_, color] : colorResult.colorOfWeb) {
            if (color >= 0) maxColorUsed = std::max(maxColorUsed, color);
        }
        outFile << "registers: " << (maxColorUsed + 1) << "\n";

        for (const auto& w : sortedWebs) {
            auto it = colorResult.colorOfWeb.find(w.id);
            int assignedColor = (it == colorResult.colorOfWeb.end()) ? -1 : it->second;
            if (assignedColor < 0) {
                outFile << "M: web" << w.id << "\n";
            } else {
                outFile << "r" << assignedColor << ": web" << w.id << "\n";
            }
        }
    } else {
        outFile << "registers: 0\n";
        for (const auto& w : sortedWebs) {
            outFile << "M: web" << w.id << "\n";
        }
    }

    std::cout << "Successfully written output to: " << filename << "\n";
}
