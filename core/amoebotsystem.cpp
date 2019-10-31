/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/amoebotsystem.h"

#include <QtGlobal>

#include "core/amoebotparticle.h"
#include <map>
#include <iostream>
#include <fstream>

AmoebotSystem::AmoebotSystem() {
  counts["round"] = new RoundCount();
  counts["activation"] = new ActivationCount();
  counts["move"] = new MoveCount();
}

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


unsigned int AmoebotSystem::numMovements() {
 return counts["move"]->value;
}

unsigned int AmoebotSystem::numRounds() {
 return counts["round"]->value;
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

void AmoebotSystem::endOfRound() {
  int roundNum = counts["round"]->value;
  for (auto const& c : counts) {
    c.second->history.push_back(c.second->value);
  }
  for (auto const& m : measures) {
    if (roundNum % m.second->frequency == 0) {
      m.second->calculate(this);
    }
  }
  counts["round"]->record();
}

void AmoebotSystem::exportData()  {
  std::ofstream outFile("//Users//josephbriones//amoebotsim/metricData.csv");

  for (auto &c : counts) {
    outFile << c.first << ", ";
    for (double roundVal : c.second->history) {
      outFile << roundVal << ", ";
    }
    outFile << std::endl;
  }

  for (auto &m : measures) {
    outFile << m.first << ", ";
    for (double roundVal : m.second->history) {
      outFile << roundVal << ", ";
    }
    outFile << std::endl;
  }

  outFile.close();
}

void AmoebotSystem::registerActivation(AmoebotParticle* particle) {
  counts["activation"]->record();
  activatedParticles.insert(particle);
  if(activatedParticles.size() == particles.size()) {
    endOfRound();
    activatedParticles.clear();
  }
}
