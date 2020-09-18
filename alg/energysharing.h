/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the Energy-Sharing
// algorithm defined in 'Bio-Inspired Energy Distribution for Programmable
// Matter' [arxiv.org/abs/2007.04377].

#ifndef ALG_ENERGYSHARING_H_
#define ALG_ENERGYSHARING_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class EnergySharingParticle : public AmoebotParticle {
 public:
  enum class Usage {
    Uniform,
    Reproduce
  };

  enum class State {
    Root,
    Idle,
    Active
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, a capacity for its
  // battery, an energy demand for its actions, an energy transfer rate, an
  // energy usage mode, and a state.
  EnergySharingParticle(const Node& head, int globalTailDir,
                        const int orientation, AmoebotSystem& system,
                        const double capacity, const double demand,
                        const double transferRate, const Usage usage,
                        const State state);

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
  EnergySharingParticle& nbrAtLabel(int label) const;

  // The three phases of the energy distribution algorithm. The communication
  // phase propagates signals communicating particles' energy levels, the
  // sharing phase gathers energy from the source and shares with a neighbor,
  // and the usage phase spends energy to perform actions, if possible.
  void communicate();
  void shareEnergy();
  void useEnergy();

  // Modifies the input color's opacity based on the particle's current battery
  // level, with a min. of 10% opacity (for little to no energy) and a max. of
  // 100% opacity (for energy >= demand).
  int energyColor(int color) const;

 protected:
  // Algorithm parameters.
  const double _capacity;
  const double _demand;
  const double _transferRate;
  const Usage _usage;

  // Local variables.
  double _battery;
  bool _stress;
  bool _inhibit;

  // Spanning tree variables.
  State _state;
  int _parentLabel;

 private:
  friend class EnergySharingSystem;
};

class EnergySharingSystem : public AmoebotSystem {
 public:
  // Constructs a system of EnergySharingParticles with an optionally specified
  // size (# particles), number of energy roots, energy usage mode (0 for
  // repeating uniform, 1 for growth), energy capacity, energy demand per
  // action, and energy transfer rate.
  EnergySharingSystem(int numParticles, const int numEnergyRoots,
                      const int usage, const double capacity,
                      const double demand, const double transferRate);
};

#endif  // ALG_ENERGYSHARING_H_
