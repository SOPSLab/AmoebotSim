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
    Q_ASSERT(particleMap.find(particle->head) == particleMap.end());
    if(particle->isExpanded()) {
        Q_ASSERT(particleMap.find(particle->tail()) == particleMap.end());
    }

    particles.push_back(particle);
    particleMap[particle->head] = particle;
    if(particle->isExpanded()) {
        particleMap[particle->tail()] = particle;
    }
}

bool AmoebotSystem::isConnected() const
{
    std::set<Node> occupiedNodes;
    for(auto p : particles) {
        occupiedNodes.insert(p->head);
        if(p->globalTailDir != -1) {
            occupiedNodes.insert(p->tail());
        }
    }
    return System::isConnected(occupiedNodes);
}
