#include <QtGlobal>

#include "sim/particle.h"

Particle::Particle(const Node _head, const int _tailDir)
    : head(_head), globalTailDir(_tailDir)
{

}

Particle::~Particle()
{
}

bool Particle::isContracted() const
{
    return globalTailDir == -1;
}

bool Particle::isExpanded() const
{
    return !isContracted();
}

Node Particle::tail() const
{
    Q_ASSERT(isExpanded());
    return head.nodeInDir(globalTailDir);
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
