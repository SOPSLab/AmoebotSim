#ifndef ALGORITHMWITHFLAGS_H
#define ALGORITHMWITHFLAGS_H

#include <array>
#include <type_traits>

#include "alg/algorithm.h"

template <class FlagClass>
class AlgorithmWithFlags : public Algorithm
{
public:
    AlgorithmWithFlags();
    AlgorithmWithFlags(const AlgorithmWithFlags& other);

    const Flag* flagAt(const int i) const;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);
    virtual Movement execute() = 0;

    void updateOutFlags();

protected:
    std::array<const FlagClass*, 10> inFlags;
    std::array<FlagClass, 10> outFlags;
};

class Flag {
public:
    Flag();
    Flag(const Flag& other);

    bool isExpanded() const;
    bool isContracted() const;

public:
    int dir;
    int tailDir;
    bool fromHead;
};

template <class FlagClass>
AlgorithmWithFlags<FlagClass>::AlgorithmWithFlags()
{
    Q_ASSERT((std::is_base_of<Flag, FlagClass>::value));
    for(auto it = inFlags.begin(); it != inFlags.end(); ++it) {
        *it = nullptr;
    }
}

template <class FlagClass>
AlgorithmWithFlags<FlagClass>::AlgorithmWithFlags(const AlgorithmWithFlags &other) :
    Algorithm(other),
    outFlags(other.outFlags)
{
    Q_ASSERT((std::is_base_of<Flag, FlagClass>::value));
    for(auto it = inFlags.begin(); it != inFlags.end(); ++it) {
        *it = nullptr;
    }
}

template <class FlagClass>
const Flag* AlgorithmWithFlags<FlagClass>::flagAt(const int i) const
{
    return &outFlags.at(i);
}

template <class FlagClass>
Movement AlgorithmWithFlags<FlagClass>::execute(std::array<const Flag*, 10>& flags)
{
    for(int i = 0; i < 10; i++) {
        inFlags[i] = (const FlagClass*) flags[i];
    }

    auto m = execute();
    updateTailDir(m);
    updateOutFlags();
    return m;
}

template <class FlagClass>
void AlgorithmWithFlags<FlagClass>::updateOutFlags()
{
    int labelLimit = isExpanded() ? 10 : 6;
    for(int label = 0; label < labelLimit; label++) {
        outFlags[label].dir = labelToDir(label);
        outFlags[label].tailDir = tailDir();
        outFlags[label].fromHead = isHeadLabel(label);
    }
}

inline Flag::Flag()
{
}

inline Flag::Flag(const Flag& other)
    : dir(other.dir),
      tailDir(other.tailDir),
      fromHead(other.fromHead)
{
}

inline bool Flag::isExpanded() const
{
    return (tailDir != -1);
}

inline bool Flag::isContracted() const
{
    return !isExpanded();
}

#endif // ALGORITHMWITHFLAGS_H
