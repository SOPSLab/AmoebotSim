/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the Leader Election
// by Erosion algorithm, originally introduced by Di Luna et al. (2020,
// https://doi.org/10.1007/s00446-019-00350-6) and later revised and extended to
// 3D space by Briones et al. (2023, https://doi.org/10.1145/3571306.3571389).
// This simulation is for the 2D version. Note that this implementation uses
// states instead of boolean flags for clarity, but is functionally equivalent
// to the pseudocode in Briones et al. (2023). Note also that the State::Null
// state is technically unneeded in the 2D version, but we implement it anyway
// for consistency with Briones et al. (2023).

#ifndef AMOEBOTSIM_ALG_LEADERELECTIONBYEROSION_H_
#define AMOEBOTSIM_ALG_LEADERELECTIONBYEROSION_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class LeaderElectionByErosionParticle : public AmoebotParticle {
 public:
  enum class State {
    Null,       // Initial state.
    Candidate,  // Non-eroded leader candidate.
    Eroded,     // Eroded, no longer a candidate.
    Leader      // The unique leader.
  };

  // Constructs a new contracted, State::Null particle with a node position for
  // its head and a particle system it belongs to.
  LeaderElectionByErosionParticle(const Node head, AmoebotSystem& system);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted.
  int headMarkColor() const override;
  int tailMarkColor() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  LeaderElectionByErosionParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing counterclockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Determines if a candidate can erode based on the erosion rules.
  bool canErode() const;

 protected:
  // Particle memory.
  State _state;

 private:
  friend class LeaderElectionByErosionSystem;
};

class LeaderElectionByErosionSystem : public AmoebotSystem {
 public:
  // Constructs a system of LeaderElectionByErosionParticles with an optionally
  // specified size (#particles) in the shape of a hexagon, since this algorithm
  // cannot handle holes.
  LeaderElectionByErosionSystem(int numParticles = 91);

  // Checks whether the system has completed leader election, i.e., there exists
  // a particle in State::Leader.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_LEADERELECTIONBYEROSION_H_
