#include "alg/energyshape.h"

#include <algorithm>
#include <set>

EnergyShapeParticle::EnergyShapeParticle(const Node& head, int globalTailDir,
                                         const int orientation,
                                         AmoebotSystem& system,
                                         const double capacity,
                                         const double demand,
                                         const double transferRate,
                                         const EnergyState eState,
                                         const ShapeState sState)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      _capacity(capacity),
      _demand(demand),
      _transferRate(transferRate),
      _battery(0),
      _stress(false),
      _inhibit(false),
      _prune(false),
      _eState(eState),
      _parentLabel(-1),
      _lastParent(0),
      _sState(sState),
      _constructionDir(-1),
      _moveDir(-1),
      _followDir(-1) {
  if (_sState == ShapeState::Seed) {
    _constructionDir = 0;
  }
}

void EnergyShapeParticle::activate() {
  if (_eState == EnergyState::Idle) {
    // Search for a root or active neighbor that does not have its prune flag
    // set in a round-robin fashion. If such a neighbor exists, set it as this
    // particle's parent and become active.
    auto prop = [&](const EnergyShapeParticle& p) {
      for (auto state : {EnergyState::Root, EnergyState::Active}) {
        if (p._eState == state && !p._prune) {
          return true;
        }
      }
      return false;
    };

    int nbrLabel = labelOfFirstNbrWithProperty<EnergyShapeParticle>(
                     prop, _lastParent);
    if (nbrLabel != -1) {
      _eState = EnergyState::Active;
      _parentLabel = nbrLabel;
      _lastParent = _parentLabel;
    }
  } else if (_prune) {
    // Pass the prune signal on to this particle's children and then prune.
    prune();
  } else {
    // Do energy distribution and perform shape formation actions.
    communicate();
    shareEnergy();
    useEnergy();
  }
}

int EnergyShapeParticle::headMarkColor() const {
  if (_eState == EnergyState::Root) {
    return energyColor(0x000000);
  } else if (_stress) {
    return energyColor(0xff0000);
  } else if (_inhibit) {
    return energyColor(0xfcd703);
  } else if (_eState == EnergyState::Active) {
    return energyColor(0x00ff00);
  } else {  // _state == State::Idle
    return -1;
  }
}

int EnergyShapeParticle::headMarkDir() const {
  return _parentLabel == -1 ? -1 : labelToDir(_parentLabel);
}

int EnergyShapeParticle::tailMarkColor() const {
  return headMarkColor();
}

QString EnergyShapeParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info (Energy Dist.):\n";
  text += "  energy state: ";
  text += [this](){
    switch(_eState) {
      case EnergyState::Root:   return "root\n";
      case EnergyState::Idle:   return "idle\n";
      case EnergyState::Active: return "active\n";
    }
    return "no state\n";
  }();
  text += "  parentLabel: " + QString::number(_parentLabel) + "\n";
  text += "  battery: " + QString::number(_battery) + " / "
                        + QString::number(_capacity) + "\n";
  text += "  stress: " + QString::number(_stress) + "\n";
  text += "  inhibit: " + QString::number(_inhibit) + "\n";
  text += "  prune: " + QString::number(_prune) + "\n\n";
  text += "Local Info (Shape Form.):\n";
  text += "  shape state: ";
  text += [this](){
    switch(_sState) {
      case ShapeState::Seed:   return "seed\n";
      case ShapeState::Idle:   return "idle\n";
      case ShapeState::Follow: return "follow\n";
      case ShapeState::Lead:   return "lead\n";
      case ShapeState::Finish: return "finish\n";
    }
    return "no state\n";
  }();
  text += "  constructDir: " + QString::number(_constructionDir) + "\n";
  text += "  moveDir: " + QString::number(_moveDir) + "\n";
  text += "  followDir: " + QString::number(_followDir) + "\n";

  return text;
}

EnergyShapeParticle& EnergyShapeParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<EnergyShapeParticle>(label);
}

void EnergyShapeParticle::prune() {
  int labelLimit = isContracted() ? 6 : 10;
  for (int nbrLabel = 0; nbrLabel < labelLimit; nbrLabel++) {
    if (hasNbrAtLabel(nbrLabel) && nbrAtLabel(nbrLabel)._parentLabel != -1
        && pointsAtMe(nbrAtLabel(nbrLabel), nbrAtLabel(nbrLabel)._parentLabel)) {
      nbrAtLabel(nbrLabel)._prune = true;
    }
  }

  _stress = false;
  _inhibit = false;
  _prune = false;
  _parentLabel = -1;

  if (_eState != EnergyState::Root) {
    _eState = EnergyState::Idle;
  }
}

void EnergyShapeParticle::communicate() {
  bool hasStressChild = false;
  int labelLimit = isContracted() ? 6 : 10;
  for (int nbrLabel = 0; nbrLabel < labelLimit; nbrLabel++) {
    if (hasNbrAtLabel(nbrLabel) && nbrAtLabel(nbrLabel)._parentLabel != -1
        && pointsAtMe(nbrAtLabel(nbrLabel), nbrAtLabel(nbrLabel)._parentLabel)
        && nbrAtLabel(nbrLabel)._stress) {
      hasStressChild = true;
      break;
    }
  }

  if (_eState != EnergyState::Root) {
    _stress = _battery < _demand || hasStressChild;
    _inhibit = nbrAtLabel(_parentLabel)._inhibit;
  } else {
    _inhibit = _battery < _demand || hasStressChild;
  }
}

void EnergyShapeParticle::shareEnergy() {
  // Root particles first harvest from the source.
  if (_eState == EnergyState::Root) {
    _battery = std::min(_battery + _transferRate, _capacity);
  }

  // All particles attempt to share energy if they have sufficient energy.
  if (_battery >= _transferRate) {
    // Find all children that do not have full batteries.
    std::vector<int> needyChildLabels;
    int labelLimit = isContracted() ? 6 : 10;
    for (int nbrLabel = 0; nbrLabel < labelLimit; nbrLabel++) {
      if (hasNbrAtLabel(nbrLabel) && nbrAtLabel(nbrLabel)._parentLabel != -1
          && pointsAtMe(nbrAtLabel(nbrLabel), nbrAtLabel(nbrLabel)._parentLabel)
          && nbrAtLabel(nbrLabel)._battery < _capacity) {
        needyChildLabels.push_back(nbrLabel);
      }
    }
    // If there is a child with a non-full battery, share with one at random.
    if (!needyChildLabels.empty()) {
      int childLabel = needyChildLabels[randInt(0, needyChildLabels.size())];
      auto child = nbrAtLabel(childLabel);
      _battery -= std::min(_transferRate, _capacity - child._battery);
      nbrAtLabel(childLabel)._battery = std::min(child._battery + _transferRate,
                                                 _capacity);
    }
  }
}

void EnergyShapeParticle::useEnergy() {
  if (!_inhibit && _battery >= _demand) {
    bool didAction = false;

    // Perform an activation of shape formation.
    if (isExpanded()) {
      if (_sState == ShapeState::Follow) {
        if (!hasNbrInState({ShapeState::Idle}) && !hasTailFollower()) {
          prune();
          _lastParent = labelToDir(_lastParent);
          contractTail();
          didAction = true;
        }
      } else if (_sState == ShapeState::Lead) {
        if (!hasNbrInState({ShapeState::Idle}) && !hasTailFollower()) {
          prune();
          _lastParent = labelToDir(_lastParent);
          contractTail();
          updateMoveDir();
          didAction = true;
        }
      }
    } else {  // is contracted.
      if (_sState == ShapeState::Idle) {
        if (hasNbrInState({ShapeState::Seed, ShapeState::Finish})) {
          _sState = ShapeState::Lead;
          updateMoveDir();
          didAction = true;
        } else if (hasNbrInState({ShapeState::Lead, ShapeState::Follow})) {
          _sState = ShapeState::Follow;
          _followDir = labelOfFirstNbrInState({ShapeState::Lead,
                                               ShapeState::Follow});
          didAction = true;
        }
      } else if (_sState == ShapeState::Follow) {
        if (hasNbrInState({ShapeState::Seed, ShapeState::Finish})) {
          _sState = ShapeState::Lead;
          updateMoveDir();
          didAction = true;
        } else if (hasTailAtLabel(_followDir)) {
          auto nbr = nbrAtLabel(_followDir);
          int nbrContractDir = nbrDirToDir(nbr, (nbr.tailDir() + 3) % 6);
          nbrAtLabel(_followDir).prune();  // DO NOT USE nbr.prune()!
          nbrAtLabel(_followDir)._lastParent = nbr.labelToDir(nbr._lastParent);
          prune();
          _lastParent = labelToDirAfterExpansion(_lastParent, _followDir);
          push(_followDir);
          _followDir = nbrContractDir;
          didAction = true;
        }
      } else if (_sState == ShapeState::Lead) {
        if (canFinish()) {
          _sState = ShapeState::Finish;
          updateConstructionDir();
          didAction = true;
        } else {
          updateMoveDir();
          if (!hasNbrAtLabel(_moveDir)) {
            prune();
            _lastParent = labelToDirAfterExpansion(_lastParent, _moveDir);
            expand(_moveDir);
          } else if (hasTailAtLabel(_moveDir)) {
            auto nbr = nbrAtLabel(_moveDir);
            nbrAtLabel(_moveDir).prune();  // DO NOT USE nbr.prune()!
            nbrAtLabel(_moveDir)._lastParent = nbr.labelToDir(nbr._lastParent);
            prune();
            _lastParent = labelToDirAfterExpansion(_lastParent, _moveDir);
            push(_moveDir);
          }
          didAction = true;
        }
      }
    }

    if (didAction) {
      _battery -= _demand;
      system.getCount("# Actions").record();
    }
  }
}

int EnergyShapeParticle::energyColor(int color) const {
  // Parse the color into RGB values.
  int r = color >> 16;
  int g = (color >> 8) % 256;
  int b = color % 256;

  // Compute opacity.
  double opacity = (std::exp(_battery - _demand) - 1) /
                   (std::exp(_battery - _demand) + 1) + 1;
  opacity = std::max(std::min(opacity, 1.0), 0.1);

  // Compute interpolation.
  r = 255 + opacity * (r - 255);
  g = 255 + opacity * (g - 255);
  b = 255 + opacity * (b - 255);

  // Return the int form of the new color.
  return (((r << 8) + g) << 8) + b;
}

int EnergyShapeParticle::labelOfFirstNbrInState(
    std::initializer_list<ShapeState> states, int startLabel) const {
  auto prop = [&](const EnergyShapeParticle& p) {
    for (auto state : states) {
      if (p._sState == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<EnergyShapeParticle>(prop, startLabel);
}

bool EnergyShapeParticle::hasNbrInState(
    std::initializer_list<ShapeState> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

int EnergyShapeParticle::constructionReceiveDir() const {
  auto prop = [&](const EnergyShapeParticle& p) {
    return isContracted()
        && (p._sState == ShapeState::Seed || p._sState == ShapeState::Finish)
        && pointsAtMe(p, p._constructionDir);
  };

  return labelOfFirstNbrWithProperty<EnergyShapeParticle>(prop);
}

bool EnergyShapeParticle::canFinish() const {
  return constructionReceiveDir() != -1;
}

void EnergyShapeParticle::updateConstructionDir() {
  _constructionDir = constructionReceiveDir();
  if (nbrAtLabel(_constructionDir)._sState == ShapeState::Seed) {
    _constructionDir = (_constructionDir + 1) % 6;
  } else {
    _constructionDir = (_constructionDir + 2) % 6;
  }

  if (hasNbrAtLabel(_constructionDir) &&
      nbrAtLabel(_constructionDir)._sState == ShapeState::Finish) {
    _constructionDir = (_constructionDir + 1) % 6;
  }
}

void EnergyShapeParticle::updateMoveDir() {
  _moveDir = labelOfFirstNbrInState({ShapeState::Seed, ShapeState::Finish});
  while (hasNbrAtLabel(_moveDir)
         && (nbrAtLabel(_moveDir)._sState == ShapeState::Seed
             || nbrAtLabel(_moveDir)._sState == ShapeState::Finish)) {
    _moveDir = (_moveDir + 5) % 6;
  }
}

bool EnergyShapeParticle::hasTailFollower() const {
  auto prop = [&](const EnergyShapeParticle& p) {
    return p._sState == ShapeState::Follow &&
           pointsAtMyTail(p, p.dirToHeadLabel(p._followDir));
  };

  return labelOfFirstNbrWithProperty<EnergyShapeParticle>(prop) != -1;
}

EnergyShapeSystem::EnergyShapeSystem(const int numParticles,
                                     const int numEnergyRoots,
                                     const double holeProb,
                                     const double capacity,
                                     const double demand,
                                     const double transferRate) {
  _counts.push_back(new Count("# Actions"));

  // Insert the energy distribution root/shape formation seed at (0,0).
  std::set<Node> occupied;
  insert(new EnergyShapeParticle(Node(0, 0), -1, randDir(), *this, capacity,
                                 demand, transferRate,
                                 EnergyShapeParticle::EnergyState::Idle,
                                 EnergyShapeParticle::ShapeState::Seed));
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add all other particles.
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
      insert(new EnergyShapeParticle(randCand, -1, randDir(), *this, capacity,
                                     demand, transferRate,
                                     EnergyShapeParticle::EnergyState::Idle,
                                     EnergyShapeParticle::ShapeState::Idle));
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

  // Choose particles at random to make energy ditribution roots.
  // BUG: If holeProb is large (e.g., > 0.7), then not all n particles will be
  // instantiated in the system. That will cause the particles[large index] call
  // to seg-fault and AmoebotSim to crash. This is an issue with all system
  // constructors that use the random tree algorithm.
  std::vector<int> indices;
  for (int i = 0; i < numParticles; ++i) {
    indices.push_back(i);
  }
  shuffle(indices.begin(), indices.end());
  for (int i = 0; i < numEnergyRoots; ++i) {
    auto esp = dynamic_cast<EnergyShapeParticle*>(particles[indices[i]]);
    esp->_eState = EnergyShapeParticle::EnergyState::Root;
  }
}

bool EnergyShapeSystem::hasTerminated() const {
  for (auto p : particles) {
    auto esp = dynamic_cast<EnergyShapeParticle*>(p);
    if (esp->_stress || esp->_inhibit ||
        (esp->_sState != EnergyShapeParticle::ShapeState::Seed
         && esp->_sState != EnergyShapeParticle::ShapeState::Finish)) {
      return false;
    }
  }

  return true;
}
