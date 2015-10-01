#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

AmoebotSystem::AmoebotSystem()
    : _numMovements(0),
      _numRounds(0)
{

}

AmoebotSystem::~AmoebotSystem()
{
    for(auto p : particles) {
        delete p;
    }
    particles.clear();
}

void AmoebotSystem::activate()
{
    if(shuffledParticles.empty()) {
        for(auto p : particles) {
            shuffledParticles.push_back(p);
        }
        shuffle(shuffledParticles.begin(), shuffledParticles.end());
    }

    shuffledParticles.front()->activate();
    registerActivation(shuffledParticles.front());
    shuffledParticles.pop_front();
}

void AmoebotSystem::activateParticleAt(Node node)
{
    auto it = particleMap.find(node);
    if(it != particleMap.end()) {
        it->second->activate();
        registerActivation(it->second);
    }
}

unsigned int AmoebotSystem::size() const
{
    return particles.size();
}

const Particle& AmoebotSystem::at(int i) const
{
    return *particles.at(i);
}

int AmoebotSystem::numMovements() const
{
    return _numMovements;
}

int AmoebotSystem::numRounds() const
{
    return _numRounds;
}

void AmoebotSystem::insert(AmoebotParticle* particle)
{
    Q_ASSERT(particleMap.find(particle->head) == particleMap.end());
    Q_ASSERT(!particle->isExpanded() || particleMap.find(particle->tail()) == particleMap.end());

    particles.push_back(particle);
    particleMap[particle->head] = particle;
    if(particle->isExpanded()) {
        particleMap[particle->tail()] = particle;
    }
}

void AmoebotSystem::registerMovement(int num)
{
    _numMovements += num;
}

void AmoebotSystem::registerActivation(AmoebotParticle* particle)
{
    activatedParticles.insert(particle);
    if(activatedParticles.size() == particles.size()) {
        _numRounds++;
        activatedParticles.clear();
    }
}
