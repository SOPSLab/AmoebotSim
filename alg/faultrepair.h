// Defines the particle system and composing particles for the Fault Repair
// Algorithm created by Kevin Lough for his undergraduate honors thesis.
//
// Run with faultrepair(#particles, hole probability) on the simulator command
// line.
//
// TODO: Choose a reasonable finite number and count upto that number of such
//      particles as you pass over them. Reset the count if the surface vector
//      changes. This would give you an idea of the scale of a fault, which past
//      a certain point could be ignored. A parameter could be added to the algorithm
//      that determines how large a concavity has to be to not be consider a fault.

#ifndef AMOEBOTSIM_ALG_FAULTREPAIR_H
#define AMOEBOTSIM_ALG_FAULTREPAIR_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class FaultRepairParticle : public AmoebotParticle {
 public:
  enum class State {
    Idle,      // Initial state.
    Follower,  // Active but not yet on the surface.
    Root,      // Moving along the surface.
    Finished   // Finished.
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

 protected:
  // Particle memory.
  State state;
  int delta1, delta2, surfaceVec, moveDir;

 private:
  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNeighborAtLabel() first if unsure.
  FaultRepairParticle& nbrAtLabel(int label) const;

  // labelOfFirstNbrInState returns the label of the first port incident to a
  // neighboring particle in any of the specified states, starting at the
  // (optionally) specified label and continuing counterclockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Checks whether this particle has an immediate child in the spanning tree
  // following its tail (if expanded) or head (if contracted).
  bool hasFollowerChild() const;

  // Returns true if the particle at the given port is an object or finished.
  bool nbrIsObject(int port) const;

  // Functions for checking conditions related to tunnels. isInTunnel() checks
  // if a particle has its object/finished neighbors in two distinct connected
  // components, and isAtDeadEnd() checks if a particle has exactly five object/
  // finished neighbors.
  bool isInTunnel() const;
  bool isAtDeadEnd() const;

  // Returns the direction to the next object surface position in a
  // counterclockwise traversal.
  int nextSurfaceDir() const;

  friend class FaultRepairSystem;
};

class FaultRepairSystem : public AmoebotSystem {
 public:
  // Constructs a system of FaultRepairParticles connected to a randomly
  // generated surface. Takes an optionally specified size (#particles) and a
  // hole probability. holeProb in [0,1] controls how "spread out" the system
  // is; closer to 0 is more compressed, closer to 1 is more expanded.
  FaultRepairSystem(uint numParticles = 100, double holeProb = 0.2);

  // Checks whether or not the system has completed fault repair (all particles
  // are contracted and in the finished state).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_FAULTREPAIR_H
