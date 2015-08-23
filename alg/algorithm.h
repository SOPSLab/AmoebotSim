#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <array>
#include <chrono>
#include <random>
#include <vector>
#include <memory>

#include <QtGlobal>

#include "sim/movement.h"

class Flag;

class Algorithm
{
public:
    Algorithm();
    Algorithm(const Algorithm& other);
    virtual ~Algorithm();

    virtual Movement execute(std::array<const Flag*, 10>& flags) = 0;
    virtual std::shared_ptr<Algorithm> blank() = 0;
    virtual std::shared_ptr<Algorithm> clone() = 0;
    virtual bool isDeterministic() const = 0;

    virtual const Flag* flagAt(const int i) const = 0;

protected:
    void updateTailDir(const Movement m);

    static int randInt(const int from, const int toNotIncluding);
    static int randDir();
    int randLabel() const;
    static float randFloat(const float from, const float toNotIncluding);
    static bool randBool(const double trueProb = 0.5);

    int labelToDir(const int label) const;
    int labelToDirAfterExpansion(const int label, const int expansionDir) const;

    int tailDir() const;
    bool isExpanded() const;
    bool isContracted() const;

    const std::vector<int>& headLabels() const;
    const std::vector<int>& tailLabels() const;
    bool isHeadLabel(const int label) const;
    bool isTailLabel(const int label) const;
    int dirToHeadLabel(const int dir) const;
    int dirToTailLabel(const int dir) const;
    int headContractionLabel() const;
    int tailContractionLabel() const;

    const std::vector<int>& headLabelsAfterExpansion(const int expansionDir) const;
    const std::vector<int>& tailLabelsAfterExpansion(const int expansionDir) const;
    bool isHeadLabelAfterExpansion(const int label, const int expansionDir) const;
    bool isTailLabelAfterExpansion(const int label, const int expansionDir) const;
    int dirToHeadLabelAfterExpansion(const int dir, const int expansionDir) const;
    int dirToTailLabelAfterExpansion(const int dir, const int expansionDir) const;
    int headContractionLabelAfterExpansion(const int expansionDir) const;
    int tailContractionLabelAfterExpansion(const int expansionDir) const;

    const std::array<int, 3>& frontLabels() const;
    const std::array<int, 2>& sideLabelsFromHead() const;
    const std::array<int, 2>& sideLabelsFromTail() const;
    const std::array<int, 3>& backLabels() const;

private:
    static int labelToDir(int label, int tailDir);

public:
    int headMarkColor;
    int headMarkDir;
    int tailMarkColor;
    int tailMarkDir;

    // the line segments for borders can exist in 18 different positions (see res/particle.png)
    std::array<int, 18> borderColors;
    std::array<int, 6> borderPointColors;

protected:
    static std::mt19937 rng;

private:
    int _tailDir;

    static const std::vector<int> sixLabels;
    // head labels depending on tailDir
    // accessing with (tailDir + 3) mod 6 gives tail labels
    static const std::array<const std::vector<int>, 6> labels;
    // valid head contraction label / index depending on tailDir
    // accessing with (tailDir + 3) mod 6 gives tail contraction label / index
    static const std::array<int, 6> contractLabels;
    // direction of an edge depending on tailDir and label
    static const std::array<std::array<int, 10>, 6> labelDir;

    static const std::array<const std::array<int, 3>, 6>_frontLabels;
    static const std::array<const std::array<int, 2>, 6>_sideLabelsFromHead;
    static const std::array<const std::array<int, 2>, 6>_sideLabelsFromTail;
    static const std::array<const std::array<int, 3>, 6>_backLabels;
};

inline Algorithm::Algorithm()
    : headMarkColor(-1),
      headMarkDir(-1),
      tailMarkColor(-1),
      tailMarkDir(-1),
      borderColors({-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}),
      borderPointColors({-1,-1,-1,-1,-1,-1}),
      _tailDir(-1)
{
    static bool rngInitialized = false;
    if(!rngInitialized) {
        uint32_t seed;
        std::random_device device;
        if(device.entropy() == 0) {
            auto duration = std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::time_point::min();
            seed = duration.count();
        } else {
            std::uniform_int_distribution<uint32_t> dist(std::numeric_limits<uint32_t>::min(),
                                                         std::numeric_limits<uint32_t>::max());
            seed = dist(device);
        }
        rng.seed(seed);
        rngInitialized = true;
    }
}

inline Algorithm::Algorithm(const Algorithm& other)
    : headMarkColor(other.headMarkColor),
      headMarkDir(other.headMarkDir),
      tailMarkColor(other.tailMarkColor),
      tailMarkDir(other.tailMarkDir),
      borderColors(other.borderColors),
      borderPointColors(other.borderPointColors),
      _tailDir(other._tailDir)
{
}

inline Algorithm::~Algorithm()
{
}

inline int Algorithm::randInt(const int from, const int toNotIncluding)
{
    std::uniform_int_distribution<int> dist(from, toNotIncluding - 1);
    return dist(rng);
}

inline int Algorithm::randDir()
{
    return randInt(0, 6);
}

inline int Algorithm::randLabel() const
{
    if(_tailDir == -1) {
        return randInt(0, 6);
    } else {
        return randInt(0, 10);
    }
}

inline float Algorithm::randFloat(const float from, const float toNotIncluding)
{
    std::uniform_real_distribution<float> dist(from, toNotIncluding);
    return dist(rng);
}

inline bool Algorithm::randBool(const double trueProb)
{
    return (randFloat(0, 1) < trueProb);
}

inline int Algorithm::labelToDir(const int label) const
{
    return labelToDir(label, _tailDir);
}

inline int Algorithm::labelToDirAfterExpansion(const int label, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    return labelToDir(label, (expansionDir + 3) % 6);
}

inline int Algorithm::tailDir() const
{
    return _tailDir;
}

inline bool Algorithm::isExpanded() const
{
    return (_tailDir != -1);
}

inline bool Algorithm::isContracted() const
{
    return !isExpanded();
}

inline const std::vector<int>& Algorithm::headLabels() const
{
    if(_tailDir == -1) {
        return sixLabels;
    } else {
        return labels[_tailDir];
    }
}

inline const std::vector<int>& Algorithm::tailLabels() const
{
    if(_tailDir == -1) {
        return sixLabels;
    } else {
        return labels[(_tailDir + 3) % 6];
    }
}

inline bool Algorithm::isHeadLabel(const int label) const
{
    for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

inline bool Algorithm::isTailLabel(const int label) const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

inline int Algorithm::dirToHeadLabel(const int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    if(isContracted()) {
        return dir;
    } else {
        for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
            if(dir == labelToDir(*it)) {
                return *it;
            }
        }
        Q_ASSERT(false);
        return 0;
    }
}

inline int Algorithm::dirToTailLabel(const int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    if(isContracted()) {
        return dir;
    } else {
        for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
            if(dir == labelToDir(*it)) {
                return *it;
            }
        }
        Q_ASSERT(false);
        return 0;
    }
}

inline int Algorithm::headContractionLabel() const
{
    Q_ASSERT(0 <= _tailDir && _tailDir <= 5);
    return contractLabels[_tailDir];
}

inline int Algorithm::tailContractionLabel() const
{
    Q_ASSERT(0 <= _tailDir && _tailDir <= 5);
    return contractLabels[(_tailDir + 3) % 6];
}

inline const std::vector<int>& Algorithm::headLabelsAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    int tempTailDir = (expansionDir + 3) % 6;
    return labels[tempTailDir];
}

inline const std::vector<int>& Algorithm::tailLabelsAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    int tempTailDir = (expansionDir + 3) % 6;
    return labels[(tempTailDir + 3) % 6];
}

inline bool Algorithm::isHeadLabelAfterExpansion(const int label, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(auto it = headLabelsAfterExpansion(expansionDir).cbegin(); it != headLabelsAfterExpansion(expansionDir).cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

inline bool Algorithm::isTailLabelAfterExpansion(const int label, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(auto it = tailLabelsAfterExpansion(expansionDir).cbegin(); it != tailLabelsAfterExpansion(expansionDir).cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

inline int Algorithm::dirToHeadLabelAfterExpansion(const int dir, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dir && dir <= 5);
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(auto it = headLabelsAfterExpansion(expansionDir).cbegin(); it != headLabelsAfterExpansion(expansionDir).cend(); ++it) {
        if(dir == labelToDirAfterExpansion(*it, expansionDir)) {
            return *it;
        }
    }
    Q_ASSERT(false);
    return 0;
}

inline int Algorithm::dirToTailLabelAfterExpansion(const int dir, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dir && dir <= 5);
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(auto it = tailLabelsAfterExpansion(expansionDir).cbegin(); it != tailLabelsAfterExpansion(expansionDir).cend(); ++it) {
        if(dir == labelToDirAfterExpansion(*it, expansionDir)) {
            return *it;
        }
    }
    Q_ASSERT(false);
    return 0;
}

inline int Algorithm::headContractionLabelAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    return contractLabels[(expansionDir + 3) % 6];
}

inline int Algorithm::tailContractionLabelAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    return contractLabels[expansionDir];
}

inline const std::array<int, 3>& Algorithm::frontLabels() const
{
    Q_ASSERT(0 <= _tailDir && _tailDir <= 5);
    return _frontLabels[_tailDir];
}

inline const std::array<int, 2>& Algorithm::sideLabelsFromHead() const
{
    Q_ASSERT(0 <= _tailDir && _tailDir <= 5);
    return _sideLabelsFromHead[_tailDir];
}

inline const std::array<int, 2>& Algorithm::sideLabelsFromTail() const
{
    Q_ASSERT(0 <= _tailDir && _tailDir <= 5);
    return _sideLabelsFromTail[_tailDir];
}

inline const std::array<int, 3>& Algorithm::backLabels() const
{
    Q_ASSERT(0 <= _tailDir && _tailDir <= 5);
    return _backLabels[_tailDir];
}

inline int Algorithm::labelToDir(int label, int tailDir)
{
    Q_ASSERT(-1 <= tailDir && tailDir <= 5);
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label <= 5);
        return label;
    } else {
        Q_ASSERT(0 <= label && label <= 9);
        return labelDir[tailDir][label];
    }
}

inline void Algorithm::updateTailDir(const Movement m)
{
    if(m.type == MovementType::Expand) {
        _tailDir = (m.label + 3) % 6;
    } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
        _tailDir = -1;
    }
}

#endif // ALGORITHM_H
