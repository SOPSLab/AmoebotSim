/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a particle system for demonstrating pull handover functionality.
//
// Run with pulldemo() on the simulator command line.

#ifndef AMOEBOTSIM_ALG_DEMO_PULLDEMO_H_
#define AMOEBOTSIM_ALG_DEMO_PULLDEMO_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class PullDemoParticle : public AmoebotParticle {
 public:
  enum class State {
    Leader,
    Follower
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  PullDemoParticle(const Node head, const int globalTailDir,
                   const int orientation, AmoebotSystem& system, State state);

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
  PullDemoParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

 protected:
  State state;
  int moveDir;

 private:
  friend class PullDemoSystem;
};

class PullDemoSystem : public AmoebotSystem {
 public:
  // Constructs a system of two PullDemoParticles (one leader and one follower).
  PullDemoSystem();

  // Because this algorithm never terminates, this simply returns false.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_DEMO_PULLDEMO_H_
