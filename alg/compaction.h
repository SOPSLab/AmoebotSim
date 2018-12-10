// Defines the particle system and composing particles for the Local Compression
// algorithm (previously called Compaction) defined in Josh Daymude's
// undergraduate honors thesis, 'Compression in Self-Organizing Particle
// Systems' [repository.asu.edu/items/37219].
//
// NOTE: Simulation does not use handover contractions, although the original
//       algorithm does.
//
// Run with compaction(#particles, hole probability) on the simulator command
// line.

#ifndef AMOEBOTSIM_ALG_COMPACTION_H
#define AMOEBOTSIM_ALG_COMPACTION_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class CompactionParticle : public AmoebotParticle {
 public:
  enum class State {
    Seed,
    Inactive,
    Active,
    Leader,
    Follower
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  CompactionParticle(const Node head, const int globalTailDir,
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
  CompactionParticle& nbrAtLabel(int label) const;

  // Checks whether the specified port label is incident to the tail of an
  // expanded particle entirely in the neighborhood of this particle.
  bool hasExpNbrTailAtLabel(int label) const;

  // Checks whether this particle has an expanded neighbor.
  bool hasExpNbr() const;

  // labelOfFirstNbrInState() returns the label of the first port incident to a
  // neighboring particle in any of the specified states, starting at the
  // (optionally) specified label and continuing clockwise. Returns -1 if there
  // is no such neighbor.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Finds the label of the first unoccupied position in this particle's
  // neighborhood, starting from the specified label and fanning out left/right.
  int labelOfFirstUnoccupied(int startLabel) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Checks whether this particle has any children in the spanning tree pointing
  // at its tail with their followDirs (active/follower/leader).
  bool hasChild() const;

  // numNbrs() counts the number of unique neighbors (expanded neighbors whose
  // head and tail are both in the neighborhood are counted only once).
  // numAdjNbrs() similarly counts unique neighbors, but groups them by
  // connected components and returns the size of each group.
  int numNbrs() const;
  std::vector<int> numAdjNbrs() const;

  // Checks whether (i) this particle does not have exactly five neighbors, and
  // (ii) this particle's neighbors form a connected induced subgraph.
  bool isParticleCompressed() const;

 protected:
  // Particle memory.
  State state;
  int followDir;

 private:
  friend class CompactionSystem;
};

class CompactionSystem : public AmoebotSystem {
 public:
  // Constructs a system of CompactionParticles with an optionally specified
  // size (#particles) and hole probability. holeProb in [0,1] controls how
  // "spread out" the system is; closer to 0 is more compressed, closer to 1 is
  // more expanded.
  CompactionSystem(int numParticles = 100, double holeProb = 0.4);

  // Because this algorithm never terminates, this simply returns false.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_COMPACTION_H
