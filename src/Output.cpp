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

    // 1. Output the Webs section
    outFile << "# Total number of webs followed by the listing of the program points of each one\n";
    outFile << "# program points in each web are sorted in ascending order\n";
    outFile << "webs: " << webs.size() << "\n";

    // Ensure webs are sorted by ID to maintain deterministic and readable output
    std::vector<Web> sortedWebs = webs;
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

            // Add comma if it's not the last element
            if (i < w.points.size() - 1) {
                outFile << ",";
            }
        }
        outFile << "\n";
    }

    // 2. Output the Registers section
    outFile << "# Total number of registers used, followed by assignment to webs\n";

    if (colorResult.success) {
        int maxColorUsed = -1;
        for (const auto& webID_color : colorResult.colorOfWeb) {
            if (webID_color.second > maxColorUsed) {
                maxColorUsed = webID_color.second;
            }
        }
        int registersUsed = maxColorUsed + 1; // Since colors are 0-indexed
        outFile << "registers: " << registersUsed << "\n";

        // Loop through the sorted webs and print their assigned registers
        for (const auto& w : sortedWebs) {
            int assignedColor = -1;
            auto it = colorResult.colorOfWeb.find(w.id);
            if(it != colorResult.colorOfWeb.end()) {
                assignedColor = it->second;
            }

            // Print M if spilled/memory, otherwise print the register
            if (assignedColor == -1) {
                outFile << "M: web" << w.id << "\n";
            } else {
                outFile << "r" << assignedColor << ": web" << w.id << "\n";
            }
        }
    } else {
        // If allocation completely failed, 0 registers are used and all go to memory
        outFile << "registers: 0\n";
        for (const auto& w : sortedWebs) {
            outFile << "M: web" << w.id << "\n";
        }
    }

    outFile.close();
    std::cout << "Successfully written output to: " << filename << "\n";
}