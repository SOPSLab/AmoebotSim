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
        Collision,
        Disconnected,
        Terminated,
        Deadlock
    };

public:
    System();
    System(const System& other);
    System& operator=(const System& other);

    SystemState insert(const Particle &p);
    const Particle& at(int index) const;
    int size() const;

    SystemState round();
    SystemState deterministicRound();
    SystemState nondeterministicRound();

    SystemState getSystemState() const;

protected:
    std::array<const Flag*, 10> assembleFlags(Particle& p);
    bool handleExpansion(Particle& p, int dir);
    bool handleContraction(Particle& p, int dir, bool isHandoverContraction);
    void activateParticlesAround(Particle& p);
    bool hasBlockedParticle() const;

protected:
    std::mt19937 rng;

    std::deque<Particle> particles;
    std::deque<Particle*> activeParticles;
    std::map<Node, Particle*> particleMap;

    SystemState systemState;
};

#endif // SYSTEM_H
