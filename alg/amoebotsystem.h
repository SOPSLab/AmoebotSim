// Defines a particle system that complies with the properties and constraints
// of the amoebot model, but is independent of any particular algorithm.

#ifndef AMOEBOTSIM_ALG_AMOEBOTSYSTEM_H
#define AMOEBOTSIM_ALG_AMOEBOTSYSTEM_H

#include <deque>
#include <map>
#include <set>
#include <vector>

#include "helper/randomnumbergenerator.h"
#include "sim/system.h"
#include "sim/tile.h"

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

  // Returns the number of tiles in the system.
  unsigned int numTiles() const final;

  // Returns a reference to the particle at the specified index of particles.
  const Particle& at(int i) const final;

  // Returns a reference to the tile list.
  virtual const std::deque<Tile*>& getTiles() const final;

  // Functions for measuring the progress of the system. numMovements returns
  // the total number of expansions and contractions performed by the system's
  // particles. numRounds returns the number of completed asynchronous rounds;
  // recall that an asynchronous round is complete when each particle has been
  // activated at least once.
  unsigned int numMovements() const final;
  unsigned int numRounds() const final;

  // Inserts a particle or a tile, respectively, into the system. A particle can be contracted or
  // expanded. Fails if the respective node(s) are already occupied.
  void insert(AmoebotParticle* particle);
  void insert(Tile* tile);

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

 protected:
  std::vector<AmoebotParticle*> particles;
  std::map<Node, AmoebotParticle*> particleMap;
  std::set<AmoebotParticle*> activatedParticles;
  std::deque<Tile*> tiles;
  std::map<Node, Tile*> tileMap;


  unsigned int _numMovements;
  unsigned int _numRounds;
};

#endif  // AMOEBOTSIM_ALG_AMOEBOTSYSTEM_H
