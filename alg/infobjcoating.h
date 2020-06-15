/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the Infinite Object
// Coating Algorithm as defined in 'Infinite Object Coating in the Ameobot
// Model' [arxiv.org/abs/1411.2356]. Note that the token passing scheme for
// complaining used here differs slightly from the complaint flag scheme in the
// paper, but is functionally equivalent. Also note that pull handovers are not
// used in this simulation, for simplicity in updating follower move directions.

#ifndef AMOEBOTSIM_ALG_INFOBJCOATING_H_
#define AMOEBOTSIM_ALG_INFOBJCOATING_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class InfObjCoatingParticle : public AmoebotParticle {
 public:
  enum class State {
    Inactive,  // Initial state.
    Follower,  // Not yet touched the surface.
    Leader     // Moving along the surface.
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  InfObjCoatingParticle(const Node head, const int globalTailDir,
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
  InfObjCoatingParticle& nbrAtLabel(int label) const;

  // labelOfFirstNbrInState returns the label of the first port incident to a
  // neighboring particle in any of the specified states, starting at the
  // (optionally) specified label and continuing counterclockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Returns the direction to the next object surface position in a
  // counterclockwise traversal of the surface.
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
  int moveDir;

 private:
  friend class InfObjCoatingSystem;
};

class InfObjCoatingSystem : public AmoebotSystem {
 public:
  // Constructs a system of InfObjCoatingParticles connected to a randomly
  // generated surface (with no tunnels). Takes an optionally specified size
  // (#particles) and a hole probability. holeProb in [0,1] controls how "spread
  // out" the system is; closer to 0 is more compressed, closer to 1 is more
  // expanded.
  InfObjCoatingSystem(uint numParticles = 100, double holeProb = 0.2);

  // Checks whether or not the system has completed infinite object coating (all
  // particles contracted and on the object.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_INFOBJCOATING_H_
