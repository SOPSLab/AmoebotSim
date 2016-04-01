#ifndef SYSTEM_H
#define SYSTEM_H

#include <deque>
#include <set>

#include <QMutex>

#include "sim/node.h"
#include "sim/particle.h"

class System;

class SystemIterator
{
public:
    SystemIterator(const System* system, int pos);

    bool operator!=(const SystemIterator& other) const;
    const Particle& operator*() const;
    const SystemIterator& operator++();

private:
    int _pos;
    const System* system;
};

class System
{
public:
    System();
    virtual ~System();

    virtual void activate() = 0;
    virtual void activateParticleAt(Node node) = 0;

    virtual unsigned int size() const = 0;
    virtual const Particle& at(int i) const = 0;

    SystemIterator begin() const;
    SystemIterator end() const;

    virtual int numMovements() const;
    virtual int numRounds() const;
   virtual int leaderElectionRounds() const;

     virtual int weakBounds() const;
     virtual int strongBounds() const;

    virtual bool hasTerminated() const;

protected:
    template<class ParticleContainer>
    static bool isConnected(const ParticleContainer& particles);

public:
    QMutex mutex;
};

template<class ParticleContainer>
bool System::isConnected(const ParticleContainer& particles)
{
    std::set<Node> occupied;
    for(auto p : particles) {
        occupied.insert(p->head);
        if(p->isExpanded()) {
            occupied.insert(p->tail());
        }
    }

    std::deque<Node> queue;
    queue.push_back(*occupied.begin());

    while(!queue.empty()) {
        Node n = queue.front();
        queue.pop_front();
        for(int dir = 0; dir < 6; ++dir) {
            Node neighbor = n.nodeInDir(dir);
            auto nondeIt = occupied.find(neighbor);
            if(nondeIt != occupied.end()) {
                queue.push_back(neighbor);
                occupied.erase(nondeIt);
            }
        }
    }

    return occupied.empty();
}

#endif // SYSTEM_H
