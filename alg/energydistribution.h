/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// TODO: header comment.

#ifndef ALG_ENERGYDISTRIBUTION_H_
#define ALG_ENERGYDISTRIBUTION_H_

#include <vector>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class EnergyDistributionParticle : public AmoebotParticle {
 public:
  enum class State {
    Root,
    Idle,
    Active
  };

  // TODO: comment.
  EnergyDistributionParticle(const Node& head, int globalTailDir,
                             const int orientation, AmoebotSystem& system,
                             const double harvestRate,
                             const double inhibitedRate,
                             const double capacity, const double threshold,
                             const double environmentEnergy,
                             const double GDH, const int signalSpeed,
                             const State state,
                             const int systemType);

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
  EnergyDistributionParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Phase functions. TODO: comment.
  void communicate();
  void harvestEnergy();
  void harvestRegulant();
  void produceRegulant();
  void reproduce();

  // TODO: comment. Helper functions.
  bool isOnPeriphery() const;

  // TODO: comment. Visualization helper functions
  int interpolate(int,int,double) const;
  double opacity(double,double) const;

 protected:
  // Algorithm parameters.
  const double _harvestRate;
  const double _inhibitedRate;
  const double _capacity;
  const double _threshold;
  const double _environmentEnergy;
  const double _GDH;
  const int _signalSpeed;
  const int _systemType;

  // Local variables.
  double _energyBattery;
  double _energyBuffer;
  double _regulantBattery;
  double _regulantBuffer;
  bool _stress;
  bool _inhibit;
  int _signalTimer;

  // Spanning tree variables.
  State _state;
  int _parentDir;
  std::set<int> _childrenDirs;

 private:
  friend class EnergyDistributionSystem;
};

class EnergyDistributionSystem : public AmoebotSystem {

 public:
  EnergyDistributionSystem(int numParticles, int systemType,
                           const double harvestRate,const double inhibitedRate,
                           const double capacity, const double threshold,
                           const double environmentEnergy, const double GDH,
                           const int signalSpeed);
};


#endif  // ALG_ENERGYDISTRIBUTION_H_
