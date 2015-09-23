#ifndef LEGACYSYSTEM_H
#define LEGACYSYSTEM_H

#include <array>
#include <deque>
#include <map>
#include <random>
#include <set>

#include "alg/legacy/algorithm.h"
#include "alg/legacy/legacyparticle.h"
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
    virtual const LegacyParticle& at(const int i) const;

    virtual int numMovements() const;
    virtual int numRounds() const;

    virtual bool hasTerminated() const;

    void insertParticle(const LegacyParticle& p);

protected:
    std::array<const Flag*, 10> assembleFlags(LegacyParticle& p);
    bool handleExpansion(LegacyParticle& p, int label);
    bool handleContraction(LegacyParticle& p, int label, bool isHandoverContraction);
    void updateNumRounds(LegacyParticle* p);

protected:
    std::deque<LegacyParticle> particles;
    std::map<Node, LegacyParticle*> particleMap;
    std::set<LegacyParticle*> activatedParticles;
    std::deque<LegacyParticle*> shuffledParticles;

    std::mt19937 rng;

    SystemState systemState;

    bool checkConnectivity;

    unsigned int numNonStaticParticles;
    int _numMovements;
    int _numRounds;
};

#endif // LEGACYSYSTEM_H
