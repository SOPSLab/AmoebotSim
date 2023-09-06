/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the energy-
// constrained version of Hexagon-Formation produced by the energy distribution
// framework of Weber et al. (2023). This implementation uses the Daymude et al.
// (2021, https://doi.org/10.4230/LIPIcs.DISC.2021.20) algorithm implemented in
// hexagonformation.h. For simplicity, all actions have uniform energy demands
// in this implementation.

#ifndef AMOEBOTSIM_ALG_EDFHEXAGONFORMATION_H_
#define AMOEBOTSIM_ALG_EDFHEXAGONFORMATION_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class EDFHexagonFormationParticle : public AmoebotParticle {
 public:
  enum class EnergyState {
    Source,   // Particles with access to an external energy source.
    Idle,     // Particles outside the spanning forest.
    Active,   // Members of trees in the spanning forest.
    Asking,   // Relaying an "asking signal" to adopt idle neighbors.
    Growing,  // Has permission to adopt idle neighbors.
    Pruning   // The root of an unstable tree that is dissolving.
  };

  enum class ShapeState {
    Seed,      // The unique particle centering the hexagon.
    Idle,      // All other particles' initial state.
    Follower,  // Member of the spanning forest but not on the forming hexagon.
    Root,      // On the surface of the forming hexagon.
    Retired    // In the forming hexagon.
  };

  // Constructs a new contracted particle with a node position for its head and
  // a particle system it belongs to. Sets the energy distribution framework's
  // parameters and starts the particle with no parent and an empty battery.
  // For Hexagon-Formation, the particle gets an initial state (either
  // ShapeState::Seed or ShapeState::Idle).
  EDFHexagonFormationParticle(const Node head, AmoebotSystem& system,
                              const int capacity, const int transferRate,
                              const int demand, const ShapeState sState);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. headMarkDir (resp., tailMarkDir) returns
  // the label of the port to draw the black head (resp., tail) marker on.
  int headMarkColor() const override;
  int tailMarkColor() const override;
  int headMarkDir() const override;
  int tailMarkDir() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  EDFHexagonFormationParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing counterclockwise.
  int labelOfFirstNbrInState(std::initializer_list<EnergyState> states,
                             int startLabel = 0) const;
  int labelOfFirstNbrInState(std::initializer_list<ShapeState> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<EnergyState> states) const;
  bool hasNbrInState(std::initializer_list<ShapeState> states) const;

  /* Energy distribution framework functions. */

  // Returns a list of labels which uniquely address any children neighbors in
  // the energy distribution spanning forest.
  const std::vector<int> eChildLabels() const;

  // Sets its children's EnergyStates to pruning, resets its own and its
  // children's parent pointers, and resets its EnergyState to idle if it is not
  // a source.
  void prune();

  // Modifies the input color's opacity based on the particle's current battery
  // level, with a min. of 25% opacity (for little to no energy) and a max. of
  // 100% opacity (for energy >= demand).
  int energyColor(int color) const;

  /* Hexagon-Formation functions. */

  // Returns the direction from this particle's head to the next position in an
  // oriented traversal (+1 for clockwise, -1 for counter-clockwise) along the
  // forming hexagon's surface. See the .cpp implementation for details on why
  // the usual +1 for counter-clockwise, -1 for clockwise is reversed here.
  int nextHexagonDir(int orientation) const;

  // Returns true if and only if a seed or retired neighbor is pointing at this
  // particle with its _dir variable.
  bool canRetire() const;

  // Returns true if and only if there is a neighbor whose _sParent variable
  // points at this particle's tail.
  bool hasTailChild() const;

  // Returns a list of labels that uniquely address any contracted neighbors
  // whose _sParent variable points at this particle's tail.
  const std::vector<int> conTailChildLabels() const;

 protected:
  // Energy distribution framework parameters.
  const int _capacity;
  const int _transferRate;
  const int _demand;

  // Energy distribution framework variables.
  EnergyState _eState;
  int _eParentLabel;
  double _battery;

  // Hexagon-Formation variables.
  ShapeState _sState;
  int _sParentDir;
  int _hexagonDir;

 private:
  friend class EDFHexagonFormationSystem;
};

class EDFHexagonFormationSystem : public AmoebotSystem {
 public:
  // Constructs a system of EDFHexagonFormationParticles with an optionally
  // specified size (#particles), number of energy source particles, hole
  // probability in [0,1) controlling how sparse the initial configuration is,
  // and the energy distribution framework parameters.
  EDFHexagonFormationSystem(int numParticles = 200, int numEnergySources = 1,
                            double holeProb = 0.2, int capacity = 10,
                            int transferRate = 1, int demand = 5);

  // Checks whether all particles belong to the energy distribution spanning
  // forest, all particles have fully recharged, and the system has formed a
  // hexagon (i.e., all particles are in ShapeState::Retired).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_EDFHEXAGONFORMATION_H_
