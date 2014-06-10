#ifndef SYSTEM_H
#define SYSTEM_H

#include <array>
#include <deque>
#include <map>
#include <random>

#include "alg/algorithm.h"
#include "sim/node.h"
#include "sim/particle.h"

class System
{
public:
    System();
    System(const System& other);
    System& operator=(const System& other);

    bool insert(const Particle& p);
    const Particle& at(int index) const;
    int size() const;

    void round();

protected:
    std::array<const Flag*, 10> assembleFlags(Particle& p);
    bool handleExpansion(Particle& p, int dir);
    bool handleContraction(Particle& p, int dir, bool isHandoverContraction);

protected:
    std::mt19937 rng;

    std::deque<Particle> particles;
    std::map<Node, Particle&> particleMap;
};

#endif // SYSTEM_H
