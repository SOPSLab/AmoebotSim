#include "alg/algorithm.h"
#include "sim/particle.h"

Particle::Particle(const int _orientation, const Vec pos, const int _tailDr)
    : orientation(_orientation),
      headPos(pos),
      tailDir(_tailDr),
      algorithm(nullptr)
{
}

Particle::Particle(const Particle& other)
    : orientation(other.orientation),
      headPos(other.headPos),
      tailDir(other.tailDir),
      algorithm(nullptr)
{
    if(other.algorithm != nullptr) {
        algorithm = other.algorithm->clone();
    }
}

Particle::~Particle()
{
    delete algorithm;
}

Particle& Particle::operator=(const Particle& other)
{
    delete algorithm;

    orientation = other.orientation;
    headPos = other.headPos;
    tailDir = other.tailDir;
    if(other.algorithm != nullptr) {
        algorithm = other.algorithm->clone();
    } else {
        algorithm = nullptr;
    }

    return *this;
}

void Particle::setAlgorithm(Algorithm* _algorithm)
{
    delete algorithm;
    algorithm = _algorithm;
}

Movement Particle::executeAlgorithm()
{
    return algorithm->delegateExecute();
}

Vec Particle::tailPos() const
{
    return headPos.vecInDir(tailDir);
}
