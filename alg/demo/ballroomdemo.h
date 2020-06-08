/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a particle system for demonstrating pull/push handover and
// read & write functionality.
//
// Run with ballroomdemo() on the simulator command line.

#ifndef AMOEBOTSIM_ALG_DEMO_BALLROOMDEMO_H_
#define AMOEBOTSIM_ALG_DEMO_BALLROOMDEMO_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class BallroomDemoParticle : public AmoebotParticle {
 public:
  enum class State {
    Leader,
    Follower
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  BallroomDemoParticle(const Node head, const int globalTailDir,
                   const int orientation, AmoebotSystem& system, State state);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. headMarkDir returns the label of the port
  // on which the black head marker is drawn.
  int headMarkColor() const override;
  int tailMarkColor() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  BallroomDemoParticle& nbrAtLabel(int label) const;

  // Returns the label each particle's neighbor; A particle always has only one
  // neighbor.
  int labelOfNeighbor(BallroomDemoParticle* neighbor) const;

 protected:
  State state;
  BallroomDemoParticle *neighbor;

 private:
  friend class BallroomDemoSystem;
};

class BallroomDemoSystem : public AmoebotSystem {
 public:
  // Constructs a system of 50 dancing pairs.
  BallroomDemoSystem(unsigned int numPairs = 30);
};

#endif  // AMOEBOTSIM_ALG_DEMO_BALLROOMDEMO_H_
