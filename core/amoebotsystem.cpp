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

  for (auto t : objects) {
    delete t;
  }

  objects.clear();
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

unsigned int AmoebotSystem::numObjects() const {
  return objects.size();
}

const Particle& AmoebotSystem::at(int i) const {
  return *particles.at(i);
}

const std::deque<Object*>& AmoebotSystem::getObjects() const {
    return objects;
}

unsigned int AmoebotSystem::numMovements() const {
  return _numMovements;
}

unsigned int AmoebotSystem::numRounds() const {
  return _numRounds;
}

void AmoebotSystem::insert(AmoebotParticle* particle) {
  Q_ASSERT(particleMap.find(particle->head) == particleMap.end());
  Q_ASSERT(objectMap.find(particle->head) == objectMap.end());
  Q_ASSERT(!particle->isExpanded() ||
           particleMap.find(particle->tail()) == particleMap.end());

  particles.push_back(particle);
  particleMap[particle->head] = particle;
  if (particle->isExpanded()) {
    particleMap[particle->tail()] = particle;
  }
}

void AmoebotSystem::insert(Object* object) {
  Q_ASSERT(objectMap.find(object->_node) == objectMap.end());
  Q_ASSERT(particleMap.find(object->_node) == particleMap.end());

  objects.push_back(object);
  objectMap[object->_node] = object;
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
