/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the Ballroom code
// tutorial, demonstrating inter-particle coordination. This tutorial covers
// read/write functionality and pull/push handovers. The pseudocode is
// available in the docs:
// [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#ballroomdemo-working-together].

#ifndef AMOEBOTSIM_ALG_PA_H_
#define AMOEBOTSIM_ALG_PA_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class PAParticle : public AmoebotParticle {
    public:
     enum class State {
       Leader,
       Follower,
       Silly
     };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  PAParticle(const Node& head, const int globalTailDir,
                       const int orientation, AmoebotSystem& system, State state);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering the particle's color. headMarkColor() (resp.,
  // tailMarkColor()) returns the color to be used for the ring drawn around the
  // particle's head (resp., tail) node. In this demo, the tail color simply
  // matches the head color. headMarkDir returns the label of the port
  // on which the head marker is drawn; in this demo, this points from the
  // follower dance partner to its leader.
  int headMarkColor() const override;
//  int headMarkDir() const override;
  int tailMarkColor() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  PAParticle& nbrAtLabel(int label) const;

 protected:
  // Member variables
  int _movingDir;
  State _state;

 private:
  friend class PASystem;

  bool tryToConnectNeighbor();
};

class PASystem : public AmoebotSystem {
 public:
  // Constructs a system of the specified number of PAParticles
  PASystem(unsigned int numParticles = 30);
};

#endif  // AMOEBOTSIM_ALG_PA_H_
