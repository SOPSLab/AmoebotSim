#include <QtGlobal>

#include "sim/particle.h"

Particle::Particle(const Node _head, const int _tailDir)
    : head(_head), tailDir(_tailDir)
{

}

Particle::~Particle()
{
}

Node Particle::tail() const
{
    return head.nodeInDir(tailDir);
}

int Particle::headMarkColor() const
{
    return -1;
}

int Particle::headMarkDir() const
{
    return -1;
}

int Particle::tailMarkColor() const
{
    return -1;
}

int Particle::tailMarkDir() const
{
    return -1;
}

std::array<int, 18> Particle::borderColors() const
{
    return {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
}

std::array<int, 6> Particle::borderPointColors() const
{
    return {{-1, -1, -1, -1, -1, -1}};
}
