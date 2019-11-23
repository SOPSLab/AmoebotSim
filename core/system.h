/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a base particle system and its iterators, primarily serving the
// purpose of defining functions to be overridden by subclasses.

#ifndef AMOEBOTSIM_SIM_SYSTEM_H
#define AMOEBOTSIM_SIM_SYSTEM_H

#include <QMutex>

#include <deque>
#include <set>

#include "core/node.h"
#include "core/particle.h"
#include "core/object.h"

class System;

class SystemIterator {
 public:
  // Construct an iterator for iterating over the systems' particles, initially
  // at the given position in the particle collection.
  SystemIterator(const System* system, int pos);

  // Iterator operators. != checks whether this iterator and the other are in
  // different positions. * returns a reference to the particle currently being
  // iterated over. ++ increases the iterator position by 1 and returns a
  // reference to the resulting iterator.
  bool operator!=(const SystemIterator& other) const;
  const Particle& operator*() const;
  const SystemIterator& operator++();

 private:
  int _pos;
  const System* system;
};

class System {
 public:
  // Default constructor and destructor. TODO: not sure if we need these.
  System();
  virtual ~System();

  // Signatures for functions which activate particles. Must be overridden by
  // any system subclasses; see amoebotsystem.h for more detailed documentation.
  virtual void activate() = 0;
  virtual void activateParticleAt(Node node) = 0;

  // Returns the number of particles in the system. Must be overridden by any
  // system subclasses.
  virtual unsigned int size() const = 0;

  // Returns the number of objects in the system.
  virtual unsigned int numObjects() const = 0;

  // Returns a reference to the particle at the specified index. Must be
  // overridden by any system subclasses.
  virtual const Particle& at(int i) const = 0;

  // Returns a reference to the object list.
  virtual const std::deque<Object*>& getObjects() const = 0;

  // STL-like begin and end functions for particle-accessing iterators.
  SystemIterator begin() const;
  SystemIterator end() const;

  // Signatures for functions measuring the progress of the system. These all
  // return default values at this level; see amoebotsystem.h for more detailed
  // documentation.
  virtual unsigned int numMovements() const;
  virtual unsigned int numRounds() const;

  virtual bool hasTerminated() const;

 protected:
  // Checks whether the particle system forms one connected component.
  template<class ParticleContainer>
  static bool isConnected(const ParticleContainer& particles);

 public:
  QMutex mutex;
};

template<class ParticleContainer>
bool System::isConnected(const ParticleContainer& particles) {
  std::set<Node> occupiedNodes;
  for (auto p : particles) {
    occupiedNodes.insert(p->head);
    if (p->isExpanded()) {
      occupiedNodes.insert(p->tail());
    }
  }

  std::deque<Node> queue;
  queue.push_back(*occupiedNodes.begin());
  occupiedNodes.clear(); // remove the first node already

  while (!queue.empty()) {
    Node n = queue.front();
    queue.pop_front();
    for (int dir = 0; dir < 6; ++dir) {
      Node neighbor = n.nodeInDir(dir);
      auto nondeIt = occupiedNodes.find(neighbor);
      if (nondeIt != occupiedNodes.end()) {
        queue.push_back(neighbor);
        occupiedNodes.erase(nondeIt);
      }
    }
  }

  return occupiedNodes.empty();
}

#endif  // AMOEBOTSIM_SIM_SYSTEM_H
