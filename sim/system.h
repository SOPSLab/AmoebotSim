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

protected:
    static int posMod(const int a, const int n);

protected:
    std::mt19937 rng;

    std::deque<Particle> particles;
    std::map<Vec, Particle*> particleMap;
};

inline int System::posMod(const int a, const int n)
{
    return ((a % n) + n) % n;
}

#endif // SYSTEM_H
