#ifndef SYSTEM_H
#define SYSTEM_H

#include <array>
#include <deque>
#include <map>
#include <random>

#include "alg/algorithm.h"
#include "sim/node.h"
#include "sim/particle.h"

class System
{
public:
    enum class SystemState {
        Valid,
        Disconnected,
        Terminated,
        Deadlocked
    };

public:
    System();
    System(const System& other);
    System& operator=(const System& other);

    SystemState insert(const Particle &p);
    const Particle& at(int index) const;
    int size() const;

    SystemState round();
    SystemState roundForParticle(const Node node);

    SystemState getSystemState() const;
    Node getDisconnectionNode() const;
    int getNumMovements() const;
    int getRounds() const;

protected:
    std::array<const Flag*, 10> assembleFlags(Particle& p);
    bool handleExpansion(Particle& p, int label);
    bool handleContraction(Particle& p, int label, bool isHandoverContraction);
    bool isConnected() const;

protected:
    std::mt19937 rng;

    std::deque<Particle> particles;
    std::map<Node, Particle*> particleMap;

    SystemState systemState;
    Node disconnectionNode;

    int numMovements;
    int rounds;
};

#endif // SYSTEM_H
