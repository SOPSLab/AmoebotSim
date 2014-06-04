#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <array>
#include <vector>

#include "sim/movement.h"

class Flag {
public:
    Flag();
    Flag(const Flag& other);

    bool expanded;
};

class Algorithm
{
public:
    Algorithm();
    Algorithm(const Algorithm& other);
    virtual ~Algorithm();

    Movement delegateExecute(std::array<const Flag*, 10>& flags);

    virtual Algorithm* clone() = 0;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags) = 0;

    template<class T> void initFlags();
    template<class T> void copyFlags(const Algorithm& algorithm);
    void deleteFlags();
    template<class T> std::array<const T*, 10> castFlags(std::array<const Flag*, 10>& flags);

public:
    std::array<Flag*, 10> outFlags;

    bool expanded;
    std::vector<int> headLabels;
    std::vector<int> tailLabels;
    int headContractLabel;
    int tailContractLabel;

    int headColor;
    int tailColor;
};

template<class T> void Algorithm::initFlags()
{
    for(auto it = outFlags.begin(); it != outFlags.end(); ++it) {
        *it = new T();
    }
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

template<class T> std::array<const T*, 10> Algorithm::castFlags(std::array<const Flag*, 10>& flags)
{
    std::array<const T*, 10> inFlags;
    for(decltype(flags.size()) i = 0; i < flags.size(); i++) {
        inFlags[i] = (T*)flags[i];
    }
    return inFlags;
}

#endif // ALGORITHM_H
