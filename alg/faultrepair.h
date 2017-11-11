// Run with faultRepair(#particles, hole probability) on the simulator command
// line.
// ADD: Make object particle that were active particles recognizable, and then
//      choose a reasonabel finite number and count upto that number of such
//      particles as you pass over them. Reset the count if the surface vector
//      changes. This would give you an idea of the scale of a fault, which past
//      a certain point could be ignored.

#ifndef AMOEBOTSIM_ALG_FAULTREPAIR_H
#define AMOEBOTSIM_ALG_FAULTREPAIR_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class FaultRepairParticle : public AmoebotParticle {
 public:
  enum class State {
    Object,    // Special state for object particles.
    Inactive,  // Initial state.
    Follower,  // Not yet touched the surface.
    Leader     // Moving along the surface.
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  FaultRepairParticle(const Node head, const int globalTailDir,
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
  // hasNeighborAtLabel() first if unsure.
  FaultRepairParticle& nbrAtLabel(int label) const;

  // labelOfFirstNbrInState returns the label of the first port incident to a
  // neighboring particle in any of the specified states, starting at the
  // (optionally) specified label and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Returns the direction to the next object surface position in a
  // counterclockwise traversal.
  int nextSurfaceDir() const;

  // Checks whether this particle has an immediate child in the spanning tree
  // following its tail (if expanded) or head (if contracted).
  bool hasFollowerChild() const;

 protected:
  // Complaint token used in stopping the leader particles from traversing the
  // object's surface forever.
  struct ComplaintToken : public Token {};

  // Particle memory.
  State state;
  int moveDir, surfaceVect, deltaOne, deltaTwo;

 private:
  friend class FaultRepairSystem;
};

class FaultRepairSystem : public AmoebotSystem {
 public:
  // Constructs a system of Particles connected to a randomly
  // generated surface (with no tunnels). Takes an optionally specified size
  // (#particles) and a hole probability. holeProb in [0,1] controls how "spread
  // out" the system is; closer to 0 is more compressed, closer to 1 is more
  // expanded.
  FaultRepairSystem(uint numParticles = 100, float holeProb = 0.2);

  // Checks whether or not the system has completed fault repair (all
  // particles contracted and in the object state.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_FAULTREPAIR_H
