/**
 * @file Structures.h
 * @brief Shared data structures for live ranges, webs, register configuration and coloring results.
 */

#ifndef PROJETO2_STRUCTURES_H
#define PROJETO2_STRUCTURES_H

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief One program point in a live range or web.
 *
 * A `+` marker in the input sets isStart, and a `-` marker sets isEnd.  A point may have
 * neither marker when it is only an intermediate point in the live range.  During web merging,
 * a point that is both start and end is normalized to an interior point because it represents
 * a fused self-assignment boundary such as `i = i + 1`.
 */
struct ProgramPoint {
    /** @brief Program line number. */
    int line = 0;
    /** @brief true when the live range starts at this line. */
    bool isStart = false;
    /** @brief true when the live range ends at this line. */
    bool isEnd = false;
};

/**
 * @brief Raw live range read from the ranges input file.
 */
struct LiveRange {
    /** @brief Variable name associated with the range. */
    std::string variable;
    /** @brief Ordered or unordered program points that form the range. */
    std::vector<ProgramPoint> points;
};

/**
 * @brief Register-allocation configuration read from the register input file.
 */
struct RegisterConfig {
    /** @brief Maximum number of physical registers allowed. */
    int numRegisters = 0;
    /** @brief Selected algorithm: basic, spilling, splitting or free. */
    std::string algorithm;
    /** @brief Optional numeric parameter used by spilling and splitting. */
    int parameter = 0;
};

/**
 * @brief A live web obtained by merging overlapping live ranges of one variable.
 */
struct Web {
    /** @brief Stable numeric identifier printed as web<id>. */
    int id = -1;
    /** @brief Source variable represented by the web. */
    std::string variable;
    /** @brief Sorted program points covered by the web. */
    std::vector<ProgramPoint> points;

    /**
     * @brief Compares webs by identifier.
     * @param other Web to compare with.
     * @return true if both webs have the same id.
     * @complexity O(1).
     */
    bool operator==(const Web& other) const {
        return id == other.id;
    }
};

/**
 * @brief Result of an allocation attempt.
 *
 * colorOfWeb maps a web id to a register number.  The value -1 means that the web is intentionally
 * allocated in memory (spilled).  When finalWebs is not empty it is the web set that must be printed,
 * which matters after web splitting creates derived webs.
 */
struct ColoringResult {
    /** @brief true when the requested allocation strategy found a valid assignment. */
    bool success = false;
    /** @brief Mapping from web id to register number, or -1 for memory. */
    std::unordered_map<int, int> colorOfWeb;
    /** @brief Final web set to print, especially after splitting. */
    std::vector<Web> finalWebs;
};
#endif //PROJETO2_STRUCTURES_H
