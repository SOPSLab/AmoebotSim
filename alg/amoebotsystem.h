#ifndef AMOEBOTSYSTEM_H
#define AMOEBOTSYSTEM_H

#include <deque>
#include <map>
#include <set>

#include "helper/randomnumbergenerator.h"
#include "sim/system.h"

class AmoebotParticle;

class AmoebotSystem : public System, public RandomNumberGenerator
{
    friend class AmoebotParticle;

public:
    AmoebotSystem();
    virtual ~AmoebotSystem();

    virtual void activate();
    virtual void activateParticleAt(Node node);

    virtual unsigned int size() const;
    virtual const Particle& at(int i) const;

    virtual int numMovements() const;
    virtual int numRounds() const;

    virtual void insert(AmoebotParticle* particle);

    void registerMovement(int num = 1);
    void registerActivation(AmoebotParticle* particle);

protected:
    std::deque<AmoebotParticle*> particles;
    std::map<Node, AmoebotParticle*> particleMap;
    std::deque<AmoebotParticle*> shuffledParticles;
    std::set<AmoebotParticle*> activatedParticles;

    int _numMovements;
    int _numRounds;
};

#endif // AMOEBOTSYSTEM_H
