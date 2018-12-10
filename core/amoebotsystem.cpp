#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

AmoebotSystem::AmoebotSystem()
  : _numMovements(0),
    _numRounds(0) {}

AmoebotSystem::~AmoebotSystem() {
  for (auto p : particles) {
    delete p;
  }

  particles.clear();

  for (auto t : tiles) {
    delete t;
  }

  tiles.clear();
}

void AmoebotSystem::activate() {
  int rand = randInt(0, particles.size());
  particles.at(rand)->activate();
  registerActivation(particles.at(rand));
}

void AmoebotSystem::activateParticleAt(Node node) {
  auto it = particleMap.find(node);
  if (it != particleMap.end()) {
    it->second->activate();
    registerActivation(it->second);
  }
}

unsigned int AmoebotSystem::size() const {
  return particles.size();
}

unsigned int AmoebotSystem::numTiles() const {
  return tiles.size();
}

const Particle& AmoebotSystem::at(int i) const {
  return *particles.at(i);
}

const std::deque<Tile*>& AmoebotSystem::getTiles() const {
    return tiles;
}

unsigned int AmoebotSystem::numMovements() const {
  return _numMovements;
}

unsigned int AmoebotSystem::numRounds() const {
  return _numRounds;
}

void AmoebotSystem::insert(AmoebotParticle* particle) {
  Q_ASSERT(particleMap.find(particle->head) == particleMap.end());
  Q_ASSERT(tileMap.find(particle->head) == tileMap.end());
  Q_ASSERT(!particle->isExpanded() ||
           particleMap.find(particle->tail()) == particleMap.end());

  particles.push_back(particle);
  particleMap[particle->head] = particle;
  if (particle->isExpanded()) {
    particleMap[particle->tail()] = particle;
  }
}

void AmoebotSystem::insert(Tile* tile) {
  Q_ASSERT(tileMap.find(tile->node) == tileMap.end());
  Q_ASSERT(particleMap.find(tile->node) == particleMap.end());

  tiles.push_back(tile);
  tileMap[tile->node] = tile;
}

void AmoebotSystem::registerMovement(unsigned int num) {
  _numMovements += num;
}

void AmoebotSystem::registerActivation(AmoebotParticle* particle) {
  activatedParticles.insert(particle);
  if(activatedParticles.size() == particles.size()) {
    _numRounds++;
    activatedParticles.clear();
  }
}
