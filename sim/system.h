#ifndef SYSTEM_H
#define SYSTEM_H

#include <deque>
#include <random>

#include "particle.h"

class System
{
public:
    System();
    System(const System* other);

    void round();

protected:
    static int posMod(const int a, const int n);

public:
    std::deque<Particle> particles;

protected:
    std::mt19937 rng;
};

inline int System::posMod(const int a, const int n)
{
    return ((a % n) + n) % n;
}

#endif // SYSTEM_H
