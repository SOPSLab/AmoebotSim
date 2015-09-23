#ifndef LEGACYSYSTEM_H
#define LEGACYSYSTEM_H

#include <array>
#include <deque>
#include <map>
#include <random>
#include <set>

#include "alg/legacy/algorithm.h"
#include "sim/system.h"

class LegacySystem : public System
{
protected:
    enum class SystemState {
        Valid,
        Disconnected,
        Terminated,
        Deadlocked
    };

public:
    LegacySystem();

    virtual void activate();

    virtual unsigned int size() const;
    virtual const Particle& at(const int i) const;

    virtual int numMovements() const;
    virtual int numRounds() const;

    void insertParticle(const Particle& p);

protected:
    std::array<const Flag*, 10> assembleFlags(Particle& p);
    bool handleExpansion(Particle& p, int label);
    bool handleContraction(Particle& p, int label, bool isHandoverContraction);
    void updateNumRounds(Particle* p);

protected:
    std::deque<Particle> particles;
    std::map<Node, Particle*> particleMap;
    std::set<Particle*> activatedParticles;
    std::deque<Particle*> shuffledParticles;

    std::mt19937 rng;

    SystemState systemState;

    bool checkConnectivity;

    unsigned int numNonStaticParticles;
    int _numMovements;
    int _numRounds;
};

#endif // LEGACYSYSTEM_H
