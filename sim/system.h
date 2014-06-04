#ifndef SYSTEM_H
#define SYSTEM_H

#include <deque>
#include <map>
#include <random>

#include "sim/particle.h"
#include "sim/vec.h"

class System
{
public:
    System();
    System(const System& other);

    bool insert(const Particle& p);
    const Particle& at(int index) const;
    int size() const;

    void round();
    bool handleExpansion(Particle& p, int dir);
    bool handleContraction(Particle& p, int dir, bool isHandoverContraction);

protected:
    template<int n> static int posMod(const int a);

protected:
    std::mt19937 rng;

    std::deque<Particle> particles;
    std::map<Vec, Particle&> particleMap;
};

template<int n> int System::posMod(const int a)
{
    return (a % n + n) % n;
}

#endif // SYSTEM_H
