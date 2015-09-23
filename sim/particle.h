#ifndef PARTICLE_H
#define PARTICLE_H

#include <array>

#include "sim/node.h"

class Particle
{
public:
    Particle(const Node _head = Node(0, 0), const int _tailDir = -1);
    virtual ~Particle();

    bool isContracted() const;
    bool isExpanded() const;

    Node tail() const;

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;
    virtual int tailMarkDir() const;

    virtual std::array<int, 18> borderColors() const;
    virtual std::array<int, 6> borderPointColors() const;

public:
    Node head;
    int globalTailDir;
};

#endif // PARTICLE_H
