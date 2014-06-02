#ifndef PARTICLE_H
#define PARTICLE_H

#include "sim/vec.h"

class Algorithm;

class Particle
{
public:
    Particle(const int _orientation, const Vec pos, const int _tailDir = -1);
    Particle(const Particle& other);
    ~Particle();

    Particle& operator=(const Particle& other);

    int orientation; // global direction
    Vec headPos;
    int tailDir; // global direction

    Algorithm* algorithm;
};

#endif // PARTICLE_H
