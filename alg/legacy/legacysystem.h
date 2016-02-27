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
    virtual ~LegacySystem();

    virtual void activate();
    virtual void activateParticleAt(Node node);

    virtual unsigned int size() const;
    virtual const LegacyParticle& at(int i) const;

    virtual int numMovements() const;
    virtual int numRounds() const;
   virtual int leaderElectionRounds() const;

    virtual int weakBounds() const;
    virtual int strongBounds() const;
    void setWeakBound(int bound);
    void setStrongBound(int bound);

    virtual bool hasTerminated() const;

    void insertParticle(const LegacyParticle& p);

protected:
    std::array<const Flag*, 10> assembleFlags(LegacyParticle& p);
    bool handleExpansion(LegacyParticle& p, int label);
    bool handleContraction(LegacyParticle& p, int label, bool isHandoverContraction);
    void updateNumRounds(LegacyParticle* p);
    template<int n> static int posMod(int a);

protected:
    std::deque<LegacyParticle*> particles;
    std::map<Node, LegacyParticle*> particleMap;
    std::set<LegacyParticle*> activatedParticles;
    std::deque<LegacyParticle*> shuffledParticles;

    std::mt19937 rng;

    SystemState systemState;

    unsigned int numNonStaticParticles;
    int _numMovements;
    int _numRounds;
    int _leRounds;
    int weakBound;
    int strongBound;
};

template<int n> int LegacySystem::posMod(int a)
{
    return (a % n + n) % n;
}

#endif // LEGACYSYSTEM_H
