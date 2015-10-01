#include <QtGlobal>

#include "sim/particle.h"

Particle::Particle(const Node head, int globalTailDir)
    : head(head), globalTailDir(globalTailDir)
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
}

Particle::~Particle()
{

}

bool Particle::isContracted() const
{
    return (globalTailDir == -1);
}

bool Particle::isExpanded() const
{
    return !isContracted();
}

Node Particle::tail() const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    return head.nodeInDir(globalTailDir);
}

int Particle::headMarkColor() const
{
    return -1;
}

int Particle::headMarkGlobalDir() const
{
    return -1;
}

int Particle::tailMarkColor() const
{
    return -1;
}

int Particle::tailMarkGlobalDir() const
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

QString Particle::inspectionText() const
{
    return "Overwrite Particle::inspectionText\nto specify an inspection text.";
}
