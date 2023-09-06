/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/leaderelectionbyerosion.h"

LeaderElectionByErosionParticle::LeaderElectionByErosionParticle(
  const Node head, AmoebotSystem &system)
    : AmoebotParticle(head, -1, randDir(), system),
      _state(State::Null) {}

void LeaderElectionByErosionParticle::activate() {
  if (_state == State::Null) {  // "Setup" action.
    _state = State::Candidate;
  } else if (_state == State::Candidate
             && !hasNbrInState({State::Null})
             && canErode()) {  // "Erode" action.
    _state = State::Eroded;
  } else if (_state == State::Candidate
             && !hasNbrInState({State::Null, State::Candidate})
             ) {  // "DeclareLeader" action.
    _state = State::Leader;
  }
}

int LeaderElectionByErosionParticle::headMarkColor() const {
  switch(_state) {
    case State::Null:      return -1;
    case State::Candidate: return 0x0000ff;
    case State::Eroded:    return 0x333333;
    case State::Leader:    return 0x00ff00;
    default:               return -1;
  }
}

int LeaderElectionByErosionParticle::tailMarkColor() const {
  return headMarkColor();
}

QString LeaderElectionByErosionParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
          + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  state: ";
  text += [this](){
    switch(_state) {
      case State::Null:      return "null candidate\n";
      case State::Candidate: return "candidate\n";
      case State::Eroded:    return "eroded\n";
      case State::Leader:    return "leader\n";
      default:               return "no state\n";
    }
  }();

  return text;
}

LeaderElectionByErosionParticle& LeaderElectionByErosionParticle::nbrAtLabel(
    int label) const {
  return AmoebotParticle::nbrAtLabel<LeaderElectionByErosionParticle>(label);
}

int LeaderElectionByErosionParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const LeaderElectionByErosionParticle& p) {
    for (auto state : states) {
      if (p._state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<LeaderElectionByErosionParticle>(
      prop, startLabel);
}

bool LeaderElectionByErosionParticle::hasNbrInState(
    std::initializer_list<State> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

bool LeaderElectionByErosionParticle::canErode() const {
  // First, count the number of candidate neighbors.
  uint numCandNbrs = 0;
  for (int label : uniqueLabels()) {
    if (hasNbrAtLabel(label) && nbrAtLabel(label)._state == State::Candidate)
      ++numCandNbrs;
  }

  // Rule 1: Return true if there is exactly one candidate neighbor.
  if (numCandNbrs == 1)
    return true;

  // Otherwise, determine if the candidate neighbors form a connected component.
  if (numCandNbrs > 0) {
    // Find any candidate neighbor.
    int candLabel = labelOfFirstNbrInState({State::Candidate});
    std::set<int> connectedCandLabels = {candLabel};

    // Sweep counter-clockwise from this candidate, stopping when an unoccupied
    // position or non-candidate neighbor is encountered. Note that it's okay in
    // this particular case to hardcode the upper limit of 6 (distinct) labels
    // since particles are instantiated as contracted and never move.
    for (uint offset = 1; offset < 6; ++offset) {
      int label = (candLabel + offset) % 6;
      if (hasNbrAtLabel(label) && nbrAtLabel(label)._state == State::Candidate){
        connectedCandLabels.insert(label);
      } else {
        break;
      }
    }

    // Then do the same but in the clockwise direction.
    for (uint offset = 1; offset < 6; ++offset) {
      int label = (candLabel - offset + 6) % 6;
      if (hasNbrAtLabel(label) && nbrAtLabel(label)._state == State::Candidate){
        connectedCandLabels.insert(label);
      } else {
        break;
      }
    }

    // Rule 2: Return true if there are 2 to 5 candidate neighbors that form a
    // connected component.
    return numCandNbrs >= 2 && numCandNbrs <= 5
           && numCandNbrs == connectedCandLabels.size();
  }

  // Otherwise, this particle cannot erode.
  return false;
}

LeaderElectionByErosionSystem::LeaderElectionByErosionSystem(int numParticles) {
  int x, y;
  for (int i = 1; i <= numParticles; ++i) {
    int layer = 1;
    int position = i - 1;
    while (position - (6 * layer) >= 0) {
      position -= 6 * layer;
      ++layer;
    }

    switch(position / layer) {
      case 0: {
        x = layer;
        y = (position % layer) - layer;
        if (position % layer == 0) {x -= 1; y += 1;}  // Corner case.
        break;
      }
      case 1: {
        x = layer - (position % layer);
        y = position % layer;
        break;
      }
      case 2: {
        x = -1 * (position % layer);
        y = layer;
        break;
      }
      case 3: {
        x = -1 * layer;
        y = layer - (position % layer);
        break;
      }
      case 4: {
        x = (position % layer) - layer;
        y = -1 * (position % layer);
        break;
      }
      case 5: {
        x = (position % layer);
        y = -1 * layer;
        break;
      }
    }

    insert(new LeaderElectionByErosionParticle(Node(x, y), *this));
  }
}

bool LeaderElectionByErosionSystem::hasTerminated() const {
  for (auto p : particles) {
    auto lep = dynamic_cast<LeaderElectionByErosionParticle*>(p);
    if (lep->_state == LeaderElectionByErosionParticle::State::Leader)
      return true;
  }

  return false;
}
