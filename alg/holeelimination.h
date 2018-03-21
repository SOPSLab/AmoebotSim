// Defines the particle system and composing particles for the Hole Elimination
// algorithm defined in Josh Daymude's undergraduate honors thesis, 'Compression
// in Self-Organizing Particle Systems' [repository.asu.edu/items/37219].
//
// Run with holeelimination(#particles, hole probability) on the simulator
// command line.

#ifndef AMOEBOTSIM_ALG_HOLEELIMINATION_H
#define AMOEBOTSIM_ALG_HOLEELIMINATION_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class HoleEliminationParticle : public AmoebotParticle {
 public:
  enum class State {
    Seed,
    Inactive,
    Walking,
    Follower,
    Finished
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  HoleEliminationParticle(const Node head, const int globalTailDir,
                          const int orientation, AmoebotSystem& system,
                          State state);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. headMarkDir returns the label of the port
  // on which the black head marker is drawn.
  int headMarkColor() const override;
  int headMarkDir() const override;
  int tailMarkColor() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  HoleEliminationParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise. Returns -1 if there is no such neighbor.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Returns the label of the first port incident to a neighboring follower or
  // walking particle whose parent in the spanning tree is this particle,
  // starting at the (optionally) specified label and continuing clockwise.
  // Returns -1 if there is no such child.
  int labelOfFirstChild(int startLabel = 0) const;

  // Updates this particle's moveDir when it is walking to traverse the finished
  // structure clockwise.
  void updateMoveDir();

  // Returns the label of the port incident to a finished neighbor whose axisDir
  // is pointing at this particle's position as the next axis position; returns
  // -1 if no such neighbor exists.
  int axisReceiveDir() const;

  // Checks whether this particle has three consecutive finished neighbor
  // particles.
  int adjFinishedNbrsDir() const;


 protected:
  State state;
  int axisDir;
  int moveDir;

 private:
  friend class HoleEliminationSystem;
};

class HoleEliminationSystem : public AmoebotSystem {
 public:
  // Constructs a system of HoleEliminationParticles with an optionally
  // specified size (#particles) and hole probability. holeProb in [0,1]
  // controls how "spread out" the system is; closer to 0 is more compressed,
  // closer to 1 is more expanded.
  HoleEliminationSystem(uint numParticles = 100, float holeProb = 0.4);

  // Checks whether or not the system's run of the hole elimination algorithm
  // has terminated (all particles in state Finished).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_HOLEELIMINATION_H
