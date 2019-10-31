/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a particle system that complies with the properties and constraints
// of the amoebot model, but is independent of any particular algorithm.

#ifndef AMOEBOTSIM_CORE_AMOEBOTSYSTEM_H_
#define AMOEBOTSIM_CORE_AMOEBOTSYSTEM_H_

#include <deque>
#include <map>
#include <set>
#include <vector>

#include "core/object.h"
#include "core/system.h"
#include "core/metric.h"
#include "helper/randomnumbergenerator.h"

// AmoebotParticle must be forward declared to avoid a cyclic dependency.
class AmoebotParticle;

class AmoebotSystem : public System, public RandomNumberGenerator {
  friend class AmoebotParticle;

 public:
  // Constructs a new particle system with zero movements and zero rounds.
  AmoebotSystem();

  // Deletes the particles in this system before destructing the system.
  virtual ~AmoebotSystem();

  // Functions for activating a particle in the system. activate activates a
  // random particle in the system, while activateParticleAt activates the
  // particle occupying the specified node if such a particle exists.
  // ISSUE: it appears that activate simply shuffles the particles and then
  // executes them in order, meaning each particle goes exactly once per round.
  // This seems inconsistent with our model, where no assumptions are made about
  // particles' relative computational speeds. Also see registerActivation.
  void activate() final;
  void activateParticleAt(Node node) final;

  // Returns the number of particles in the system.
  unsigned int size() const final;

  // Returns the number of objects in the system.
  unsigned int numObjects() const final;

  // Returns a reference to the particle at the specified index of particles.
  const Particle& at(int i) const final;

  // Returns a reference to the object list.
  virtual const std::deque<Object*>& getObjects() const final;

  // Functions for measuring the progress of the system. numMovements returns
  // the total number of expansions and contractions performed by the system's
  // particles. numRounds returns the number of completed asynchronous rounds;
  // recall that an asynchronous round is complete when each particle has been
  // activated at least once.
  unsigned int numMovements();
  unsigned int numRounds();

  // Inserts a particle or a object, respectively, into the system. A particle can be contracted or
  // expanded. Fails if the respective node(s) are already occupied.
  void insert(AmoebotParticle* particle);
  void insert(Object* object);

  // Functions for logging the progress of the system. registerMovement
  // increments the total number of movements the system has made by the given
  // amount. registerActivation logs that the given particle has been activated;
  // when all particles have been activated at least once, it resets its logging
  // and increments the number of completed asynchronous rounds by one.
  // ISSUE: registerActivation simply inserts particles into a collection and
  // increments the rounds when the collection size equals the system size,
  // which relies on the assumption that #activations per round = #particles.
  // See activate.
  void registerMovement(unsigned int num = 1);
  void registerActivation(AmoebotParticle* particle);

  // Function for signaling the end of a round, triggering the recording of
  // counts and possible calculation of measures.
  void endOfRound();

  // Triggers the export of all metrics' history, written into a .csv file.
  void exportData();

 protected:
  std::vector<AmoebotParticle*> particles;
  std::map<Node, AmoebotParticle*> particleMap;
  std::set<AmoebotParticle*> activatedParticles;
  std::deque<Object*> objects;
  std::map<Node, Object*> objectMap;
  std::map<std::string, Count*> counts;
  std::map<std::string, Measure*> measures;
};

class ActivationCount : public Count {
public:
  ActivationCount() {
    value = 0;
    name = "activation";
  }
  ~ActivationCount() {}
  void record() {
    value++;
  }
};

class RoundCount : public Count {
public:
  RoundCount() {
    value = 0;
    name = "round";
  }
  ~RoundCount() {}
  void record() {
    value++;
  }
};

class MoveCount : public Count {
public:
  MoveCount() {
    value = 0;
    name = "moves";
  }
  ~MoveCount() {}
  void record() {
    value++;
  }
};

#endif  // AMOEBOTSIM_CORE_AMOEBOTSYSTEM_H_



