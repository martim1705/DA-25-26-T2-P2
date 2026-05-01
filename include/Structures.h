#ifndef PROJETO2_STRUCTURES_H
#define PROJETO2_STRUCTURES_H

#include <string>
#include <unordered_map>
#include <vector>

struct ProgramPoint {
    int line;
    bool isStart = false;
    bool isEnd = false;
};

struct LiveRange {
    std::string variable;
    std::vector<ProgramPoint> points;
};

struct RegisterConfig {
    int numRegisters = 0;
    std::string algorithm;
    int parameter = 0;
};

struct Web {
    int id;
    std::string variable;
    std::vector<ProgramPoint> points;

    bool operator==(const Web& other) const {
        return id == other.id;
    }
};

struct ColoringResult {
    bool success;
    std::unordered_map<int, int> colorOfWeb;
};
#endif //PROJETO2_STRUCTURES_H
