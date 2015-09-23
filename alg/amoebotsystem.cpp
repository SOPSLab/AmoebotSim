#include "amoebotsystem.h"

AmoebotSystem::AmoebotSystem()
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
    shuffledParticles.pop_front();
}

unsigned int AmoebotSystem::size() const
{
    return particles.size();
}

const Particle& AmoebotSystem::at(const int i) const
{
    return *particles.at(i);
}

void AmoebotSystem::insert(AmoebotParticle* particle)
{
    particles.push_back(particle);
    particleMap[particle->head] = particle;
    if(particle->isExpanded()) {
        particleMap[particle->tail()] = particle;
    }
}
