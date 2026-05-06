#include "Parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
// lê registers.txt

namespace {
    std::string trim(std::string s) {
        auto notSpace = [](unsigned char c) {
            return !std::isspace(c);
        };

        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());

        return s;
    }
}

//  # comment line
//  registers: 1
//  # algorithm variants: basic, splitting and spilling each with a numeric parameter
//  algorithm: basic

RegisterConfig parseRegistersFile(const std::string& filename) {
    RegisterConfig config;
    config.numRegisters = 0;
    config.algorithm = "";
    config.parameter = 0;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << filename << " could not be opened.\n";
        return config;
    }
    std::string line;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        // registers: <-
        size_t pos = line.find(':');
        // if no ":" is found -> register file is bad configured
        if (pos == std::string::npos) {
            std::cerr << "Invalid config line: " << line << std::endl;
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

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

    }
    return config;
}

//  # this line is just a comment to be ignored
//  sum: 7+,8,9,10-
//  i: 1+,2,3,4,5,6-
//  i: 9+,10,11,12-
//  i: 12+,13,14-
//  i: 20+,11,12-

// Pega no texto neste formator e cria um vetor de
// LiveRanges (Ver Structures.h)
// Útil para criar as webs mais tarde

std::vector<LiveRange> parseRangesFile(const std::string& filename) {
    std::vector<LiveRange> rangesFile;


    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << filename << " could not be opened.\n";
        return rangesFile;
    }
    std::string line;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            std::cerr << "Invalid range line: " << line << std::endl;
            continue;
        }
        LiveRange liveRange;
        liveRange.variable = trim(line.substr(0, colonPos));
        std::string pointsText = line.substr(colonPos + 1);
        std::stringstream ss(pointsText);
        std::string token;


        while(getline(ss, token, ',')) {
            token = trim(token);
            ProgramPoint point;
            point.isStart = false;
            point.isEnd = false;

            if (!token.empty() && token.back() == '+') {
                point.isStart = true;
                token.pop_back();
            } else if (!token.empty() && token.back() == '-') {
                point.isEnd = true;
                token.pop_back();
            }
            point.line = stoi(token);

            liveRange.points.push_back(point);
        }
        rangesFile.push_back(liveRange);
    }
    return rangesFile;
}