/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the composition of
// hexagon formation and Energy-Sharing as described in 'Bio-Inspired Energy
// Distribution for Programmable Matter' [arxiv.org/abs/2007.04377].

#ifndef ALG_ENERGYSHAPE_H_
#define ALG_ENERGYSHAPE_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class EnergyShapeParticle : public AmoebotParticle {
 public:
  enum class EnergyState {
    Root,
    Idle,
    Active
  };

  enum class ShapeState {
    Seed,
    Idle,
    Follow,
    Lead,
    Finish
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, a capacity for its
  // battery, an energy demand for its actions, an energy transfer rate, an
  // energy state, and a shape state.
  EnergyShapeParticle(const Node& head, int globalTailDir,
                      const int orientation, AmoebotSystem& system,
                      const double capacity, const double demand,
                      const double transferRate, const EnergyState eState,
                      const ShapeState sState);

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
  EnergyShapeParticle& nbrAtLabel(int label) const;

  // Sets all energy children's prune flags to true, resets flags in memory,
  // removes energy parent label, and resets energy state to idle (if active).
  void prune();

  /* Energy Distribution functions. */

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

  /* Shape Formation functions. */

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<ShapeState> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<ShapeState> states) const;

  // Returns the label of the port incident to a neighbor which is finished and
  // pointing at this particle's position as the next one to be filled; returns
  // -1 if such a neighbor does not exist.
  int constructionReceiveDir() const;

  // Checks whether this particle is occupying the next position to be filled.
  bool canFinish() const;

  // Sets this particle's constructionDir to point at the next position to be
  // filled as it is finishing.
  void updateConstructionDir();

  // Updates this particle's moveDir when it is a leader to traverse the current
  // surface of the forming shape counter-clockwise.
  void updateMoveDir();

  // Checks whether this particle has an immediate child in the spanning tree
  // following its tail.
  bool hasTailFollower() const;

 protected:
  // Energy Distribution parameters.
  const double _capacity;
  const double _demand;
  const double _transferRate;

  // Energy Distribution variables.
  double _battery;
  bool _stress;
  bool _inhibit;
  bool _prune;
  EnergyState _eState;
  int _parentLabel;
  int _lastParent;

  // Shape Formation variables.
  ShapeState _sState;
  int _constructionDir;
  int _moveDir;
  int _followDir;

 private:
  friend class EnergyShapeSystem;
};

class EnergyShapeSystem : public AmoebotSystem {
 public:
  // Constructs a system of EnergyShapeParticles with an optionally specified
  // size (# particles), number of energy distribution root particles, hole
  // probability in [0,1) controlling how sparse the initial configuration is,
  // energy capacity, energy demand per action, and energy transfer rate.
  EnergyShapeSystem(const int numParticles, const int numEnergyRoots,
                    const double holeProb, const double capacity,
                    const double demand, const double transferRate);

  // Checks whether the system has completed forming the desired shape (i.e.,
  // all particles are in shape state Finish).
  bool hasTerminated() const override;
};

#endif  // ALG_ENERGYSHAPE_H_
