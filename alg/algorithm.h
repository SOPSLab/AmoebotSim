#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <array>
#include <chrono>
#include <random>
#include <vector>

#include <QtGlobal>

#include "sim/movement.h"

class Flag {
public:
    Flag();
    Flag(const Flag& other);

public:
    int dir;
    int tailDir;
};

class Algorithm
{
public:
    Algorithm();
    Algorithm(const Algorithm& other);
    virtual ~Algorithm();

    Movement delegateExecute(std::array<const Flag*, 10>& flags);
    virtual Algorithm* clone() = 0;
    virtual bool isDeterministic() const = 0;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags) = 0;

    template<class T> void initFlags();
    template<class T> void copyFlags(const Algorithm& algorithm);
    void deleteFlags();
    template<class T> static std::array<const T*, 10> castFlags(std::array<const Flag*, 10>& _flags);
    template<class T> static std::array<T*, 10> castFlags(std::array<Flag*, 10>& _flags);

    template<class T> static bool contains(std::vector<T> vector, T value);

    static int randInt(const int from, const int toNotIncluding);
    static int randDir();
    int randLabel() const;
    static float randFloat(const float from, const float toNotIncluding);
    static bool randBool(const double trueProb = 0.5);

    int labelToDir(int label) const;

    int tailDir() const;
    bool isExpanded() const;
    bool isContracted() const;
    const std::vector<int>& headLabels() const;
    const std::vector<int>& tailLabels() const;
    int headContractionLabel() const;
    int tailContractionLabel() const;

    const std::array<int, 3>& backLabels() const;

private:
    static int labelToDir(int label, int tailDir);
    void updateLabels(const Movement m);
    void updateOutFlags();

public:
    std::array<Flag*, 10> outFlags;
    int headMarkColor;
    int headMarkDir;
    int tailMarkColor;
    int tailMarkDir;

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

    static const std::array<const std::array<int, 3>, 6>_backLabels;
};

inline Flag::Flag()
{
}

inline Flag::Flag(const Flag& other)
    : dir(other.dir),
      tailDir(other.tailDir)
{
}

inline Algorithm::Algorithm()
    : headMarkColor(-1),
      headMarkDir(-1),
      tailMarkColor(-1),
      tailMarkDir(-1),
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
      _tailDir(other._tailDir)
{
}

inline Algorithm::~Algorithm()
{
}

inline Movement Algorithm::delegateExecute(std::array<const Flag*, 10>& flags)
{
    Movement m = execute(flags);
    updateLabels(m);
    updateOutFlags();
    return m;
}

template<class T> void Algorithm::initFlags()
{
    for(auto it = outFlags.begin(); it != outFlags.end(); ++it) {
        *it = new T();
    }
    updateOutFlags();
}

template<class T> void Algorithm::copyFlags(const Algorithm& algorithm)
{
    for(decltype(algorithm.outFlags.size()) i = 0; i < algorithm.outFlags.size(); i++) {
        outFlags[i] = new T(*(T*)algorithm.outFlags[i]);
    }
}

inline void Algorithm::deleteFlags()
{
    for(auto it = outFlags.begin(); it != outFlags.end(); ++it) {
        delete *it;
    }
}

template<class T> std::array<const T*, 10> Algorithm::castFlags(std::array<const Flag*, 10>& _flags)
{
    std::array<const T*, 10> flags;
    for(decltype(_flags.size()) i = 0; i < _flags.size(); i++) {
        flags[i] = (T*)_flags[i];
    }
    return flags;
}

template<class T> std::array<T*, 10> Algorithm::castFlags(std::array<Flag*, 10>& _flags)
{
    std::array<T*, 10> flags;
    for(decltype(_flags.size()) i = 0; i < _flags.size(); i++) {
        flags[i] = (T*)_flags[i];
    }
    return flags;
}

template<class T> bool Algorithm::contains(std::vector<T> vector, T value)
{
    for(decltype(vector.size()) i = 0; i < vector.size(); i++) {
        if(vector[i] == value) {
            return true;
        }
    }
    return false;
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

inline int Algorithm::labelToDir(int label) const
{
    return labelToDir(label, _tailDir);
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

inline void Algorithm::updateLabels(const Movement m)
{
    if(m.type == MovementType::Expand) {
        _tailDir = (m.dir + 3) % 6;
    } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
        _tailDir = -1;
    }
}

inline void Algorithm::updateOutFlags()
{
    int labelLimit = _tailDir == -1 ? 6 : 10;
    for(int i = 0; i < labelLimit; i++) {
        outFlags[i]->dir = labelToDir(i);
        outFlags[i]->tailDir = _tailDir;
    }
}

#endif // ALGORITHM_H
