/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/edfleaderelectionbyerosion.h"

EDFLeaderElectionByErosionParticle::EDFLeaderElectionByErosionParticle(
    const Node head,
    AmoebotSystem& system,
    const int capacity,
    const int transferRate,
    const int demand)
    : AmoebotParticle(head, -1, randDir(), system),
      _capacity(capacity),
      _transferRate(transferRate),
      _demand(demand),
      _eState(EnergyState::Idle),
      _eParentDir(-1),
      _battery(0),
      _lState(LeaderState::Null) {}

void EDFLeaderElectionByErosionParticle::activate() {
  // Prioritize Leader-Election-By-Erosion actions over energy distribution.
  if (_battery >= _demand
      && _eState != EnergyState::Idle && _eState != EnergyState::Pruning
      && !hasNbrInState({EnergyState::Idle, EnergyState::Pruning})) {
    if (_lState == LeaderState::Null) {  // "Setup" action.
      _battery -= _demand;
      _lState = LeaderState::Candidate;
      return;
    } else if (_lState == LeaderState::Candidate
               && !hasNbrInState({LeaderState::Null})
               && canErode()) {  // "Erode" action.
      _battery -= _demand;
      _lState = LeaderState::Eroded;
      return;
    } else if (_lState == LeaderState::Candidate
               && !hasNbrInState({LeaderState::Null, LeaderState::Candidate})
               ) {  // "DeclareLeader" action.
      _battery -= _demand;
      _lState = LeaderState::Leader;
      return;
    }
  }

  // Compute various neighbor sets used in the EnergyDistribution action.
  std::vector<int> idleNbrLabels;
  for (int label : uniqueLabels())
    if (hasNbrAtLabel(label) && nbrAtLabel(label)._eState == EnergyState::Idle)
      idleNbrLabels.push_back(label);

  std::vector<int> askingChildLabels;
  for (int childLabel : childLabels())
    if (nbrAtLabel(childLabel)._eState == EnergyState::Asking)
      askingChildLabels.push_back(childLabel);

  std::vector<int> nonFullChildLabels;
  for (int childLabel : childLabels())
    if (nbrAtLabel(childLabel)._battery < _capacity)
      nonFullChildLabels.push_back(childLabel);

  // Execute the EnergyDistribution action if it is enabled.
  if (_eState == EnergyState::Pruning) {  // GetPruned
    prune();
  }
  if (_eState == EnergyState::Active
      && (!idleNbrLabels.empty() || !askingChildLabels.empty())) {  // AskGrowth
    _eState = EnergyState::Asking;
  }
  if ((_eState == EnergyState::Source
       && (!idleNbrLabels.empty() || !askingChildLabels.empty()))
      || _eState == EnergyState::Growing) {  // GrowForest
    // Adopt idle neighbors as active children.
    for (int label : idleNbrLabels) {
      nbrAtLabel(label)._eParentDir = dirToNbrDir(nbrAtLabel(label),
                                                  (labelToDir(label) + 3) % 6);
      nbrAtLabel(label)._eState = EnergyState::Active;
    }

    // Set asking children to growing.
    for (int label : askingChildLabels)
      nbrAtLabel(label)._eState = EnergyState::Growing;

    // Become active if growing.
    if (_eState == EnergyState::Growing)
      _eState = EnergyState::Active;
  }
  if (_eState == EnergyState::Source
      && _battery < _capacity) {  // HarvestEnergy
    _battery += _transferRate;
  }
  if (_eState != EnergyState::Idle && _eState != EnergyState::Pruning
      && _battery >= _transferRate
      && !nonFullChildLabels.empty()) {  // ShareEnergy
    _battery -= _transferRate;
    int childLabel = nonFullChildLabels[randInt(0, nonFullChildLabels.size())];
    nbrAtLabel(childLabel)._battery += _transferRate;
  }
}

int EDFLeaderElectionByErosionParticle::headMarkColor() const {
  switch(_eState) {
    case EnergyState::Source:   return energyColor(0x000000);
    case EnergyState::Idle:     return -1;
    case EnergyState::Active:   return energyColor(0x00ff00);
    case EnergyState::Asking:   return energyColor(0x00ff00);
    case EnergyState::Growing:  return energyColor(0x00ff00);
    case EnergyState::Pruning:  return energyColor(0xff0000);
    default:                    return -1;
  }

  /* switch(_lState) {
    case LeaderState::Null:      return -1;
    case LeaderState::Candidate: return 0x0000ff;
    case LeaderState::Eroded:    return 0x333333;
    case LeaderState::Leader:    return 0x00ff00;
    default:                     return -1;
  } */
}

int EDFLeaderElectionByErosionParticle::tailMarkColor() const {
  return headMarkColor();
}

int EDFLeaderElectionByErosionParticle::headMarkDir() const {
  return _eParentDir == -1 ? -1 : labelToDir(_eParentDir);
}

QString EDFLeaderElectionByErosionParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
          + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info (EDF):\n";
  text += "  state: ";
  text += [this](){
    switch(_eState) {
      case EnergyState::Source:   return "source\n";
      case EnergyState::Idle:     return "idle\n";
      case EnergyState::Active:   return "active\n";
      case EnergyState::Asking:   return "asking\n";
      case EnergyState::Growing:  return "growing\n";
      case EnergyState::Pruning:  return "pruning\n";
      default:                    return "no state\n";
    }
  }();
  text += "  parentDir: " + QString::number(_eParentDir) + "\n";
  text += "  battery: " + QString::number(_battery) + " / "
          + QString::number(_capacity) + "\n\n";
  text += "Local Info (LE):\n";
  text += "  state: ";
  text += [this](){
    switch(_lState) {
      case LeaderState::Null:      return "null candidate\n";
      case LeaderState::Candidate: return "candidate\n";
      case LeaderState::Eroded:    return "eroded\n";
      case LeaderState::Leader:    return "leader\n";
      default:                     return "no state\n";
    }
  }();

  return text;
}

EDFLeaderElectionByErosionParticle&
    EDFLeaderElectionByErosionParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<EDFLeaderElectionByErosionParticle>(label);
}

int EDFLeaderElectionByErosionParticle::labelOfFirstNbrInState(
    std::initializer_list<LeaderState> states, int startLabel) const {
  auto prop = [&](const EDFLeaderElectionByErosionParticle& p) {
    for (auto state : states) {
      if (p._lState == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<EDFLeaderElectionByErosionParticle>(
      prop, startLabel);
}

int EDFLeaderElectionByErosionParticle::labelOfFirstNbrInState(
    std::initializer_list<EnergyState> states, int startLabel) const {
  auto prop = [&](const EDFLeaderElectionByErosionParticle& p) {
    for (auto state : states) {
      if (p._eState == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<EDFLeaderElectionByErosionParticle>(
      prop, startLabel);
}

bool EDFLeaderElectionByErosionParticle::hasNbrInState(
    std::initializer_list<LeaderState> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

bool EDFLeaderElectionByErosionParticle::hasNbrInState(
    std::initializer_list<EnergyState> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

const std::vector<int> EDFLeaderElectionByErosionParticle::childLabels() const {
  std::vector<int> labels;
  for (int label : uniqueLabels())
    if (hasNbrAtLabel(label)
        && nbrAtLabel(label)._eParentDir != -1
        && pointsAtMe(nbrAtLabel(label), nbrAtLabel(label)._eParentDir))
      labels.push_back(label);

  return labels;
}

void EDFLeaderElectionByErosionParticle::prune() {
  for (int childLabel : childLabels()) {
    nbrAtLabel(childLabel)._eState = EnergyState::Pruning;
    nbrAtLabel(childLabel)._eParentDir = -1;
  }

  if (_eState != EnergyState::Source) {
    _eState = EnergyState::Idle;
  }
}

int EDFLeaderElectionByErosionParticle::energyColor(int color) const {
  // Parse the color into RGB values.
  int r = color >> 16;
  int g = (color >> 8) % 256;
  int b = color % 256;

  // Compute opacity.
  double opacity = (std::exp(_battery - _demand) - 1) /
                    (std::exp(_battery - _demand) + 1) + 1;
  opacity = std::max(std::min(opacity, 1.0), 0.25);

  // Compute interpolation.
  r = 255 + opacity * (r - 255);
  g = 255 + opacity * (g - 255);
  b = 255 + opacity * (b - 255);

  // Return the int form of the new color.
  return (((r << 8) + g) << 8) + b;
}

bool EDFLeaderElectionByErosionParticle::canErode() const {
  // First, count the number of candidate neighbors.
  uint numCandNbrs = 0;
  for (int label : uniqueLabels()) {
    if (hasNbrAtLabel(label)
        && nbrAtLabel(label)._lState == LeaderState::Candidate)
      ++numCandNbrs;
  }

  // Rule 1: Return true if there is exactly one candidate neighbor.
  if (numCandNbrs == 1)
    return true;

  // Otherwise, determine if the candidate neighbors form a connected component.
  if (numCandNbrs > 0) {
    // Find any candidate neighbor.
    int candLabel = labelOfFirstNbrInState({LeaderState::Candidate});
    std::set<int> connectedCandLabels = {candLabel};

    // Sweep counter-clockwise from this candidate, stopping when an unoccupied
    // position or non-candidate neighbor is encountered. Note that it's okay in
    // this particular case to hardcode the upper limit of 6 (distinct) labels
    // since particles are instantiated as contracted and never move.
    for (uint offset = 1; offset < 6; ++offset) {
      int label = (candLabel + offset) % 6;
      if (hasNbrAtLabel(label)
          && nbrAtLabel(label)._lState == LeaderState::Candidate){
        connectedCandLabels.insert(label);
      } else {
        break;
      }
    }

    // Then do the same but in the clockwise direction.
    for (uint offset = 1; offset < 6; ++offset) {
      int label = (candLabel - offset + 6) % 6;
      if (hasNbrAtLabel(label)
          && nbrAtLabel(label)._lState == LeaderState::Candidate){
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

EDFLeaderElectionByErosionSystem::EDFLeaderElectionByErosionSystem(
    int numParticles,
    int numEnergySources,
    int capacity,
    int transferRate,
    int demand) {
  // Create a hexagon of EnergyState::Idle particles.
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

    insert(new EDFLeaderElectionByErosionParticle(Node(x, y), *this, capacity,
                                                  transferRate, demand));
  }

  // Choose source particles uniformly at random.
  std::vector<int> indices;
  for (int i = 0; i < numParticles; ++i) {
    indices.push_back(i);
  }
  shuffle(indices.begin(), indices.end());
  for (int i = 0; i < numEnergySources; ++i) {
    auto elp = dynamic_cast<EDFLeaderElectionByErosionParticle*>(
        particles[indices[i]]);
    elp->_eState = EDFLeaderElectionByErosionParticle::EnergyState::Source;
  }
}

bool EDFLeaderElectionByErosionSystem::hasTerminated() const {
  // Check if all amoebots are in the spanning forest and have full batteries.
  for (auto p : particles) {
    auto elp = dynamic_cast<EDFLeaderElectionByErosionParticle*>(p);
    if (elp->_eState == EDFLeaderElectionByErosionParticle::EnergyState::Idle
        || elp->_eState == EDFLeaderElectionByErosionParticle::EnergyState::Pruning
        || elp->_battery < elp->_capacity)
      return false;
  }

  // Check that a leader has emerged.
  for (auto p : particles) {
    auto elp = dynamic_cast<EDFLeaderElectionByErosionParticle*>(p);
    if (elp->_lState == EDFLeaderElectionByErosionParticle::LeaderState::Leader)
      return true;
  }

  return false;
}
