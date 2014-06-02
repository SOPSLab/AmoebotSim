#ifndef SYSTEM_H
#define SYSTEM_H

#include <deque>

#include "particle.h"

class System
{
public:
    System();
    System(const System* other);

    std::deque<Particle> particles;
};

#endif // SYSTEM_H
