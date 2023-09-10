/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the energy-
// constrained version of Leader-Election-By-Erosion (leaderelectionbyerosion.h)
// produced by the energy distribution framework of Weber et al. (2023). For
// simplicity, all actions have uniform energy demands in this implementation.

#ifndef AMOEBOTSIM_ALG_EDFLEADERELECTIONBYEROSION_H_
#define AMOEBOTSIM_ALG_EDFLEADERELECTIONBYEROSION_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class EDFLeaderElectionByErosionParticle : public AmoebotParticle {
 public:
  enum class EnergyState {
    Source,   // Particles with access to an external energy source.
    Idle,     // Particles outside the spanning forest.
    Active,   // Members of trees in the spanning forest.
    Asking,   // Relaying an "asking signal" to adopt idle neighbors.
    Growing,  // Has permission to adopt idle neighbors.
    Pruning   // The root of an unstable tree that is dissolving.
  };

  enum class LeaderState {
    Null,       // Initial state.
    Candidate,  // Non-eroded leader candidate.
    Eroded,     // Eroded, no longer a candidate.
    Leader      // The unique leader.
  };

  // Constructs a new contracted, LeaderState::Null particle with a node
  // position for its head and a particle system it belongs to. Also sets the
  // energy distribution framework's parameters and starts the particle with no
  // parent and an empty battery.
  EDFLeaderElectionByErosionParticle(const Node head, AmoebotSystem& system,
                                     const int capacity, const int transferRate,
                                     const int demand);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. headMarkDir returns the label of the port
  // on which the black head marker is drawn.
  int headMarkColor() const override;
  int tailMarkColor() const override;
  int headMarkDir() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  EDFLeaderElectionByErosionParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing counterclockwise.
  int labelOfFirstNbrInState(std::initializer_list<EnergyState> states,
                             int startLabel = 0) const;
  int labelOfFirstNbrInState(std::initializer_list<LeaderState> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<EnergyState> states) const;
  bool hasNbrInState(std::initializer_list<LeaderState> states) const;

  /* Energy distribution framework functions. */

  // Returns a list of labels which uniquely address any children neighbors in
  // the energy distribution spanning forest.
  const std::vector<int> childLabels() const;

  // Sets its children's EnergyStates to pruning, resets its own and its
  // children's parent pointers, and resets its EnergyState to idle if it is not
  // a source.
  void prune();

  // Modifies the input color's opacity based on the particle's current battery
  // level, with a min. of 25% opacity (for little to no energy) and a max. of
  // 100% opacity (for energy >= demand).
  int energyColor(int color) const;

  /* Leader-Election-By-Erosion functions. */

  // Determines if a candidate can erode based on the erosion rules.
  bool canErode() const;

 protected:
  // Energy distribution framework parameters.
  const int _capacity;
  const int _transferRate;
  const int _demand;

  // Energy distribution framework variables.
  EnergyState _eState;
  int _eParentDir;
  double _battery;

  // Leader-Election-By-Erosion variables.
  LeaderState _lState;

 private:
  friend class EDFLeaderElectionByErosionSystem;
};

class EDFLeaderElectionByErosionSystem : public AmoebotSystem {
  public:
  // Constructs a system of EDFLeaderElectionByErosionParticles with an
  // optionally specified size (#particles) in the shape of a hexagon, since
  // Leader-Election-By-Erosion cannot handle holes. Also sets the number of
  // source amoebots.
  EDFLeaderElectionByErosionSystem(int numParticles = 91,
                                   int numEnergySources = 1, int capacity = 10,
                                   int transferRate = 1, int demand = 5);

  // Checks whether all particles belong to the energy distribution spanning
  // forest, all particles have fully recharged, and the system has elected a
  // leader (i.e., there exists a particle in LeaderState::Leader).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_EDFLEADERELECTIONBYEROSION_H_
