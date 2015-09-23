#ifndef SYSTEM_H
#define SYSTEM_H

#include <set>

#include <QMutex>

#include "sim/node.h"
#include "sim/particle.h"

class System
{
public:
    System();
    virtual ~System();

    virtual void activate() = 0;

    virtual unsigned int size() const = 0;
    virtual const Particle& at(const int i) const = 0;

    virtual int numMovements() const;
    virtual int numRounds() const;

protected:
    static bool isConnected(std::set<Node>& occupiedNodes);

public:
    QMutex mutex;
};

#endif // SYSTEM_H
