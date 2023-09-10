/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/edfhexagonformation.h"

EDFHexagonFormationParticle::EDFHexagonFormationParticle(
    const Node head,
    AmoebotSystem& system,
    const int capacity,
    const int transferRate,
    const int demand,
    const ShapeState sState)
    : AmoebotParticle(head, -1, randDir(), system),
      _capacity(capacity),
      _transferRate(transferRate),
      _demand(demand),
      _eState(EnergyState::Idle),
      _eParentLabel(-1),
      _battery(0),
      _sState(sState),
      _sParentDir(-1),
      _hexagonDir(sState == ShapeState::Seed ? 0 : -1) {}

void EDFHexagonFormationParticle::activate() {
  // Prioritize Leader-Election-By-Erosion actions over energy distribution.
  if (_battery >= _demand
      && _eState != EnergyState::Idle && _eState != EnergyState::Pruning
      && !hasNbrInState({EnergyState::Idle, EnergyState::Pruning})) {
    // alpha_1: Idle or follower particles with a seed or retired neighbor
    // become roots and begin traversing the hexagon's surface.
    if (isContracted()
        && (_sState == ShapeState::Idle || _sState == ShapeState::Follower)
        && hasNbrInState({ShapeState::Seed, ShapeState::Retired})) {
      _battery -= _demand;  // Spend energy.
      _sParentDir = -1;
      _sState = ShapeState::Root;
      _hexagonDir = nextHexagonDir(1);  // clockwise.
      return;
    }
    // alpha_2: idle particles with follower or root neighbors become
    // followers and join the spanning forest.
    else if (_sState == ShapeState::Idle
             && hasNbrInState({ShapeState::Follower, ShapeState::Root})) {
      _battery -= _demand;  // Spend energy.
      _sParentDir = labelOfFirstNbrInState({ShapeState::Follower,
                                            ShapeState::Root});
      _sState = ShapeState::Follower;
      return;
    }
    // alpha_3: contracted roots with no idle neighbors who are pointed at by
    // a retired or seed particle's construction direction retire.
    else if (isContracted()
             && _sState == ShapeState::Root
             && !hasNbrInState({ShapeState::Idle})
             && canRetire()) {
      _battery -= _demand;  // Spend energy.
      _hexagonDir = nextHexagonDir(-1);  // counter-clockwise.
      _sState = ShapeState::Retired;
      return;
    }
    // alpha_4: contracted roots that can expand along the surface of the
    // hexagon do so.
    else if (isContracted()
             && _sState == ShapeState::Root
             && !hasNbrAtLabel(_hexagonDir)) {
      _battery -= _demand;  // Spend energy.
      if (_eParentLabel != -1)
        _eParentLabel = dirToTailLabelAfterExpansion(_eParentLabel, _hexagonDir);
      expand(_hexagonDir);  // Expand movements don't need pruning.
      return;
    }
    // alpha_5: expanded followers and roots without idle neighbors but with a
    // tail child pull a tail child in a handover.
    else if (isExpanded()
             && (_sState == ShapeState::Follower
                 || _sState == ShapeState::Root)
             && !hasNbrInState({ShapeState::Idle})
             && !conTailChildLabels().empty()) {
      _battery -= _demand;  // Spend energy.
      if (_sState == ShapeState::Root)
        _hexagonDir = nextHexagonDir(1);  // clockwise.

      // Prune before performing the pull handover and choosing a child to pull.
      _eParentLabel = -1;
      prune();

      // Choose any contracted tail child to pull in a handover.
      int childLabel = conTailChildLabels()[0];
      auto child = nbrAtLabel(childLabel);

      // Pulling this child will make it expand, so we need to update its energy
      // parent pointer and hexagon parent pointers before pulling.
      if (child._eParentLabel != -1)
        nbrAtLabel(childLabel)._eParentLabel =
            child.dirToTailLabelAfterExpansion(child._eParentLabel,
                                               child._sParentDir);
      nbrAtLabel(childLabel)._sParentDir = dirToNbrDir(
          child, (tailDir() + 3) % 6);
      pull(childLabel);
      return;
    }
    // alpha_6: expanded followers and roots without idle neighbors or tail
    // children contract their tails.
    else if (isExpanded()
             && (_sState == ShapeState::Follower
                 || _sState == ShapeState::Root)
             && !hasNbrInState({ShapeState::Idle})
             && !hasTailChild()) {
      _battery -= _demand;  // Spend energy.
      if (_sState == ShapeState::Root)
        _hexagonDir = nextHexagonDir(1);  // clockwise.

      // Prune before contracting.
      _eParentLabel = -1;
      prune();
      contractTail();
      return;
    }
  }

  // Compute various neighbor sets used in the EnergyDistribution action.
  std::vector<int> idleNbrLabels;
  for (int label : uniqueLabels())
    if (hasNbrAtLabel(label) && nbrAtLabel(label)._eState == EnergyState::Idle)
      idleNbrLabels.push_back(label);

  std::vector<int> askingChildLabels;
  for (int childLabel : eChildLabels())
    if (nbrAtLabel(childLabel)._eState == EnergyState::Asking)
      askingChildLabels.push_back(childLabel);

  std::vector<int> nonFullChildLabels;
  for (int childLabel : eChildLabels())
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
      for (int nbrLabel : nbrAtLabel(label).uniqueLabels()) {
        if (pointsAtMe(nbrAtLabel(label), nbrLabel)) {
          nbrAtLabel(label)._eParentLabel = nbrLabel;
          break;
        }
      }
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

int EDFHexagonFormationParticle::headMarkColor() const {
  switch(_eState) {
    case EnergyState::Source:   return energyColor(0x000000);
    case EnergyState::Idle:     return -1;
    case EnergyState::Active:   return energyColor(0x00ff00);
    case EnergyState::Asking:   return energyColor(0x00ff00);
    case EnergyState::Growing:  return energyColor(0x00ff00);
    case EnergyState::Pruning:  return energyColor(0xff0000);
    default:                    return -1;
  }
}

int EDFHexagonFormationParticle::tailMarkColor() const {
  return headMarkColor();
}

int EDFHexagonFormationParticle::headMarkDir() const {
  if (_eParentLabel == -1 || !isHeadLabel(_eParentLabel)) {
    return -1;
  } else {
    return labelToDir(_eParentLabel);
  }
}

int EDFHexagonFormationParticle::tailMarkDir() const {
  if (_eParentLabel == -1 || !isTailLabel(_eParentLabel)) {
    return -1;
  } else {
    return labelToDir(_eParentLabel);
  }
}

QString EDFHexagonFormationParticle::inspectionText() const {
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
  text += "  parentLabel: " + QString::number(_eParentLabel) + "\n";
  text += "  battery: " + QString::number(_battery) + " / "
          + QString::number(_capacity) + "\n\n";
  text += "Local Info (Hex.):\n";
  text += "  state: ";
  text += [this](){
    switch(_sState) {
      case ShapeState::Seed:      return "seed\n";
      case ShapeState::Idle:      return "idle\n";
      case ShapeState::Follower:  return "follower\n";
      case ShapeState::Root:      return "root\n";
      case ShapeState::Retired:   return "retired\n";
      default:                    return "no state\n";
    }
  }();
  text += "  parentDir: " + QString::number(_sParentDir) + "\n";
  text += "  hexagonDir: " + QString::number(_hexagonDir) + "\n";

  return text;
}

EDFHexagonFormationParticle& EDFHexagonFormationParticle::nbrAtLabel(
    int label) const {
  return AmoebotParticle::nbrAtLabel<EDFHexagonFormationParticle>(label);
}

int EDFHexagonFormationParticle::labelOfFirstNbrInState(
    std::initializer_list<EnergyState> states, int startLabel) const {
  auto prop = [&](const EDFHexagonFormationParticle& p) {
    for (auto state : states) {
      if (p._eState == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<EDFHexagonFormationParticle>(
      prop, startLabel);
}

int EDFHexagonFormationParticle::labelOfFirstNbrInState(
    std::initializer_list<ShapeState> states, int startLabel) const {
  auto prop = [&](const EDFHexagonFormationParticle& p) {
    for (auto state : states) {
      if (p._sState == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<EDFHexagonFormationParticle>(
      prop, startLabel);
}

bool EDFHexagonFormationParticle::hasNbrInState(
    std::initializer_list<EnergyState> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

bool EDFHexagonFormationParticle::hasNbrInState(
    std::initializer_list<ShapeState> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

const std::vector<int> EDFHexagonFormationParticle::eChildLabels() const {
  std::vector<int> labels;
  for (int label : uniqueLabels())
    if (hasNbrAtLabel(label)
        && nbrAtLabel(label)._eParentLabel != -1
        && pointsAtMe(nbrAtLabel(label), nbrAtLabel(label)._eParentLabel))
      labels.push_back(label);

  return labels;
}

void EDFHexagonFormationParticle::prune() {
  for (int childLabel : eChildLabels()) {
    nbrAtLabel(childLabel)._eState = EnergyState::Pruning;
    nbrAtLabel(childLabel)._eParentLabel = -1;
  }

  if (_eState != EnergyState::Source) {
    _eState = EnergyState::Idle;
  }
}

int EDFHexagonFormationParticle::energyColor(int color) const {
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

int EDFHexagonFormationParticle::nextHexagonDir(int orientation) const {
  // First, find a head label that points to a seed or retired neighbor.
  int hexagonLabel;
  for (int label : headLabels()) {
    if (hasNbrAtLabel(label)
        && (nbrAtLabel(label)._sState == ShapeState::Seed
            || nbrAtLabel(label)._sState == ShapeState::Retired)) {
      hexagonLabel = label;
      break;
    }
  }

  // Next, find the label that points along the hexagon's surface in a traversal
  // with the specified orientation. Perhaps counterintuitively, this means that
  // we search from the above label in the opposite orientation for the first
  // unoccupied or non-seed/retired neighbor.
  int numLabels = isContracted() ? 6 : 10;
  while (hasNbrAtLabel(hexagonLabel)
         && (nbrAtLabel(hexagonLabel)._sState == ShapeState::Seed
             || nbrAtLabel(hexagonLabel)._sState == ShapeState::Retired))
    hexagonLabel = (hexagonLabel + orientation + numLabels) % numLabels;

  // Convert this label to a direction before returning.
  return labelToDir(hexagonLabel);
}

bool EDFHexagonFormationParticle::canRetire() const {
  auto prop = [&](const EDFHexagonFormationParticle& p) {
    return (p._sState == ShapeState::Seed || p._sState == ShapeState::Retired)
           && pointsAtMe(p, p._hexagonDir);
  };

  return labelOfFirstNbrWithProperty<EDFHexagonFormationParticle>(prop) != -1;
}

bool EDFHexagonFormationParticle::hasTailChild() const {
  auto prop = [&](const EDFHexagonFormationParticle& p) {
    return p._sParentDir != -1
           && pointsAtMyTail(p, p.dirToHeadLabel(p._sParentDir));
  };

  return labelOfFirstNbrWithProperty<EDFHexagonFormationParticle>(prop) != -1;
}

const std::vector<int> EDFHexagonFormationParticle::conTailChildLabels() const {
  std::vector<int> labels;
  for (int label : tailLabels())
    if (hasNbrAtLabel(label)
        && nbrAtLabel(label).isContracted()
        && nbrAtLabel(label)._sParentDir != -1
        && pointsAtMyTail(nbrAtLabel(label), nbrAtLabel(label)._sParentDir))
      labels.push_back(label);

  return labels;
}

EDFHexagonFormationSystem::EDFHexagonFormationSystem(int numParticles,
                                                     int numEnergySources,
                                                     double holeProb,
                                                     int capacity,
                                                     int transferRate,
                                                     int demand) {
  // Insert the shape formation seed at (0,0).
  std::set<Node> occupied;
  insert(new EDFHexagonFormationParticle(
      Node(0, 0), *this, capacity, transferRate, demand,
      EDFHexagonFormationParticle::ShapeState::Seed));
  occupied.insert(Node(0, 0));

  // Initialize the candidate positions set.
  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add all other particles using the random tree algorithm.
  int particlesAdded = 1;
  while (particlesAdded < numParticles && !candidates.empty()) {
    // Pick a random candidate node.
    int randIndex = randInt(0, candidates.size());
    Node randCand;
    for (auto cand = candidates.begin(); cand != candidates.end(); ++cand) {
      if (randIndex == 0) {
        randCand = *cand;
        candidates.erase(cand);
        break;
      } else {
        randIndex--;
      }
    }

    // With probability 1 - holeProb, add a new particle at the candidate node.
    if (randBool(1.0 - holeProb)) {
      insert(new EDFHexagonFormationParticle(
          randCand, *this, capacity, transferRate, demand,
          EDFHexagonFormationParticle::ShapeState::Idle));
      occupied.insert(randCand);
      particlesAdded++;

      // Add new candidates.
      for (int i = 0; i < 6; ++i) {
        if (occupied.find(randCand.nodeInDir(i)) == occupied.end()) {
          candidates.insert(randCand.nodeInDir(i));
        }
      }
    }
  }

  // Choose source particles uniformly at random. BUG: If holeProb is large
  // (e.g., > 0.7), then not all n particles will be instantiated in the system.
  // That will cause the particles[large index] call to seg-fault and AmoebotSim
  // to crash. This is an issue with all system constructors that use the random
  // tree algorithm.
  std::vector<int> indices;
  for (int i = 0; i < numParticles; ++i) {
    indices.push_back(i);
  }
  shuffle(indices.begin(), indices.end());
  for (int i = 0; i < numEnergySources; ++i) {
    auto ehp = dynamic_cast<EDFHexagonFormationParticle*>(particles[indices[i]]);
    ehp->_eState = EDFHexagonFormationParticle::EnergyState::Source;
  }
}

bool EDFHexagonFormationSystem::hasTerminated() const {
  // Check if all amoebots are in the spanning forest and have full batteries.
  for (auto p : particles) {
    auto ehp = dynamic_cast<EDFHexagonFormationParticle*>(p);
    if (ehp->_eState == EDFHexagonFormationParticle::EnergyState::Idle
        || ehp->_eState == EDFHexagonFormationParticle::EnergyState::Pruning
        || ehp->_battery < ehp->_capacity)
      return false;
  }

  // Check that all particles are either the seed or retired.
  for (auto p : particles) {
    auto ehp = dynamic_cast<EDFHexagonFormationParticle*>(p);
    if (ehp->_sState != EDFHexagonFormationParticle::ShapeState::Seed
        && ehp->_sState != EDFHexagonFormationParticle::ShapeState::Retired)
      return false;
  }

  return true;
}
