#ifndef PROJETO2_STRUCTURES_H
#define PROJETO2_STRUCTURES_H

#include <string>
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
};
#endif //PROJETO2_STRUCTURES_H
