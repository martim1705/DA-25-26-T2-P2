/**
 * @file Parser.cpp
 * @brief Implementation of the project input parsers.
 */

#include "Parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace {
    /**
     * @brief Removes leading and trailing whitespace from a string.
     * @param s String to trim.
     * @return Trimmed copy of the input string.
     * @complexity O(n), where n is the string length.
     */
    std::string trim(std::string s) {
        auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        return s;
    }

    /**
     * @brief Checks whether a line should be ignored by the parsers.
     * @param line Input line.
     * @return true for blank lines and comment lines starting with '#'.
     * @complexity O(n), dominated by trimming the line.
     */
    bool isCommentOrBlank(const std::string& line) {
        std::string t = trim(line);
        return t.empty() || t[0] == '#';
    }
}

RegisterConfig parseRegistersFile(const std::string& filename) {
    RegisterConfig config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: " << filename << " could not be opened.\n";
        return config;
    }

    std::string line;
    while (getline(file, line)) {
        if (isCommentOrBlank(line)) continue;

        size_t pos = line.find(':');
        if (pos == std::string::npos) {
            std::cerr << "Invalid config line: " << line << '\n';
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        try {
            if (key == "registers") {
                config.numRegisters = std::stoi(value);
            } else if (key == "algorithm") {
                size_t commaPos = value.find(',');
                if (commaPos == std::string::npos) {
                    config.algorithm = trim(value);
                    config.parameter = 0;
                } else {
                    config.algorithm = trim(value.substr(0, commaPos));
                    config.parameter = std::stoi(trim(value.substr(commaPos + 1)));
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid numeric value in config line: " << line << '\n';
        }
    }

    if (config.numRegisters < 0) {
        std::cerr << "Invalid register count. It must be non-negative.\n";
        config.numRegisters = 0;
    }
    if (config.parameter < 0) {
        std::cerr << "Invalid algorithm parameter. It must be non-negative.\n";
        config.parameter = 0;
    }

    return config;
}

std::vector<LiveRange> parseRangesFile(const std::string& filename) {
    std::vector<LiveRange> rangesFile;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: " << filename << " could not be opened.\n";
        return rangesFile;
    }

    std::string line;
    while (getline(file, line)) {
        if (isCommentOrBlank(line)) continue;

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            std::cerr << "Invalid range line: " << line << '\n';
            continue;
        }

        LiveRange liveRange;
        liveRange.variable = trim(line.substr(0, colonPos));
        if (liveRange.variable.empty()) {
            std::cerr << "Invalid range line with empty variable: " << line << '\n';
            continue;
        }

        std::string pointsText = line.substr(colonPos + 1);
        std::stringstream ss(pointsText);
        std::string token;

        while (getline(ss, token, ',')) {
            token = trim(token);
            if (token.empty()) continue;

            ProgramPoint point;
            if (token.back() == '+') {
                point.isStart = true;
                token.pop_back();
            } else if (token.back() == '-') {
                point.isEnd = true;
                token.pop_back();
            }

            token = trim(token);
            try {
                point.line = std::stoi(token);
                if (point.line < 0) {
                    std::cerr << "Ignoring negative program point in line: " << line << '\n';
                    continue;
                }
                liveRange.points.push_back(point);
            } catch (const std::exception& e) {
                std::cerr << "Ignoring invalid program point '" << token << "' in line: " << line << '\n';
            }
        }

        if (liveRange.points.empty()) {
            std::cerr << "Invalid range without program points: " << line << '\n';
            continue;
        }
        rangesFile.push_back(liveRange);
    }
    return rangesFile;
}
