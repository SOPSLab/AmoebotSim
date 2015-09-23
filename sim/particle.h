#ifndef PARTICLE_H
#define PARTICLE_H

#include <array>

#include "sim/node.h"

class Particle
{
public:
    Particle(const Node _head = Node(0, 0), const int _tailDir = -1);
    virtual ~Particle();

    Node tail() const;

    virtual int headMarkColor() const = 0;
    virtual int headMarkDir() const = 0;
    virtual int tailMarkColor() const = 0;
    virtual int tailMarkDir() const = 0;

    virtual std::array<int, 18> borderColors() const = 0;
    virtual int borderDir(const int dir) const = 0;
    virtual std::array<int, 6> borderPointColors() const = 0;
    virtual int borderPointDir(const int dir) const = 0;

public:
    Node head;
    int tailDir;
};

#endif // PARTICLE_H
