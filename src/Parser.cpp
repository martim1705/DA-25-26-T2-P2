#include "Parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
// lê registers.txt

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

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "registers") {
            config.numRegisters = std::stoi(value);
        } else if (key == "algorithm") {
            size_t commaPos = value.find(',');

            if (commaPos == std::string::npos) {
                config.algorithm = value;
                config.parameter = 0;
            }
            else {
                config.algorithm = value.substr(0, commaPos);
                config.parameter = std::stoi(value.substr(commaPos + 1));
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
// LiveRanges (Ver Strucutures.h)
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
        liveRange.variable = line.substr(0, colonPos);
        std::string pointsText = line.substr(colonPos + 1);
        std::stringstream ss(pointsText);
        std::string token;


        while(getline(ss, token, ',')) {
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