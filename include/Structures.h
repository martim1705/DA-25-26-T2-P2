#ifndef PROJETO2_STRUCTURES_H
#define PROJETO2_STRUCTURES_H

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief One program point in a live range/web.
 *
 * A '+' in the input sets isStart, and a '-' sets isEnd.  A point may have
 * neither marker when it is only an intermediate point in the live range.
 */
struct ProgramPoint {
    int line = 0;
    bool isStart = false;
    bool isEnd = false;
};

/**
 * @brief Raw live range read from the ranges input file.
 */
struct LiveRange {
    std::string variable;
    std::vector<ProgramPoint> points;
};

/**
 * @brief Register-allocation configuration read from registers input file.
 */
struct RegisterConfig {
    int numRegisters = 0;
    std::string algorithm;
    int parameter = 0;
};

/**
 * @brief A live web obtained by merging overlapping live ranges of one variable.
 */
struct Web {
    int id = -1;
    std::string variable;
    std::vector<ProgramPoint> points;

    bool operator==(const Web& other) const {
        return id == other.id;
    }
};

/**
 * @brief Result of an allocation attempt.
 *
 * colorOfWeb maps a web id to a register number.  The value -1 means that the
 * web is intentionally allocated in memory (spilled).  When finalWebs is not
 * empty it is the web set that must be printed, which matters after splitting.
 */
struct ColoringResult {
    bool success = false;
    std::unordered_map<int, int> colorOfWeb;
    std::vector<Web> finalWebs;
};
#endif //PROJETO2_STRUCTURES_H
