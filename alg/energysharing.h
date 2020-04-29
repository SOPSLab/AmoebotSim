/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// TODO: header comment.

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

  // TODO: comment.
  EnergySharingParticle(const Node& head, int globalTailDir,
                        const int orientation, AmoebotSystem& system,
                        const double capacity, const double harvestRate,
                        const double batteryFrac, const Usage usage,
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

  // Phase functions. TODO: comment.
  void communicate();
  void harvestEnergy();
  void useEnergy();

  // TODO: comment. Visualization helper functions
  int energyColor(int color1, int color2) const;

 protected:
  // Algorithm parameters.
  const double _capacity;
  const double _harvestRate;
  const double _batteryFrac;
  const double _demand;
  const Usage _usage;

  // Local variables.
  double _battery;
  double _buffer;
  bool _stress;
  bool _inhibit;

  // Spanning tree variables.
  State _state;
  int _parentDir;

 private:
  friend class EnergySharingSystem;
};

class EnergySharingSystem : public AmoebotSystem {
 public:
  EnergySharingSystem(int numParticles, const int usage, const double capacity,
                      const double harvestRate, const double batteryFrac);
};

#endif  // ALG_ENERGYSHARING_H_
