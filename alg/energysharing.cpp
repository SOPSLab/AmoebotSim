#include "alg/energysharing.h"

#include <algorithm>  // for std::min, std::max.

EnergySharingParticle::EnergySharingParticle(const Node& head,
                                             int globalTailDir,
                                             const int orientation,
                                             AmoebotSystem& system,
                                             const double capacity,
                                             const double demand,
                                             const double transferRate,
                                             const Usage usage,
                                             const State state)
    : AmoebotParticle (head, globalTailDir, orientation, system),
      _capacity(capacity),
      _demand(demand),
      _transferRate(transferRate),
      _usage(usage),
      _battery(0),
      _stress(false),
      _inhibit(false),
      _state(state),
      _parentLabel(-1) {}

void EnergySharingParticle::activate() {
  if (_state == State::Idle) {
    // Setup phase: join the spanning tree by searching for a root or active
    // neighbor. If such a neighbor exists, set it as this particle's parent and
    // become active.
    auto prop = [&](const EnergySharingParticle& p) {
      for (auto state : {State::Root, State::Active}) {
        if (p._state == state) {
          return true;
        }
      }
      return false;
    };

    int nbrLabel = labelOfFirstNbrWithProperty<EnergySharingParticle>(prop);
    if (nbrLabel != -1) {
      _state = State::Active;
      _parentLabel = nbrLabel;
    }
  } else {
    communicate();
    shareEnergy();
    useEnergy();
  }
}

int EnergySharingParticle::headMarkColor() const {
  if (_state == State::Root) {
    return energyColor(0x000000);
  } else if (_stress) {
    return energyColor(0xff0000);
  } else if (_inhibit) {
    return energyColor(0xfcd703);
  } else if (_state == State::Active) {
    return energyColor(0x00ff00);
  } else {  // _state == State::Idle
    return -1;
  }
}

int EnergySharingParticle::headMarkDir() const {
  return _parentLabel;
}

int EnergySharingParticle::tailMarkColor() const {
  return headMarkColor();
}

QString EnergySharingParticle::inspectionText() const {
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
      case State::Root:   return "root\n";
      case State::Idle:   return "idle\n";
      case State::Active: return "active\n";
    }
    return "no state\n";
  }();
  text += "  parentLabel: " + QString::number(_parentLabel) + "\n";
  text += "  battery: " + QString::number(_battery) + " / "
                        + QString::number(_capacity) + "\n";
  text += "  stress: " + QString::number(_stress) + "\n";
  text += "  inhibit: " + QString::number(_inhibit) + "\n";

  return text;
}

EnergySharingParticle& EnergySharingParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<EnergySharingParticle>(label);
}

void EnergySharingParticle::communicate() {
  bool hasStressChild = false;
  for (int nbrDir = 0; nbrDir < 6; nbrDir++) {
    if (hasNbrAtLabel(nbrDir) && nbrAtLabel(nbrDir)._stress
        && pointsAtMe(nbrAtLabel(nbrDir), nbrAtLabel(nbrDir)._parentLabel)) {
      hasStressChild = true;
      break;
    }
  }

  if (_state != State::Root) {
    _stress = _battery < _demand || hasStressChild;
    _inhibit = nbrAtLabel(_parentLabel)._inhibit;
  } else {
    _inhibit = _battery < _demand || hasStressChild;
  }
}

void EnergySharingParticle::shareEnergy() {
  // Root particles first harvest from the source.
  if (_state == State::Root) {
    _battery = std::min(_battery + _transferRate, _capacity);
  }

  // All particles attempt to share energy if they have sufficient energy.
  if (_battery >= _transferRate) {
    // Find all children that do not have full batteries.
    std::vector<int> needyChildLabels;
    for (int nbrLabel = 0; nbrLabel < 6; nbrLabel++) {
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

void EnergySharingParticle::useEnergy() {
  if (!_inhibit && _battery >= _demand) {
    if (_usage == Usage::Uniform) {
      _battery -= _demand;
      system.getCount("# Actions").record();
    } else if (_usage == Usage::Reproduce) {
      int reproduceDir = -1;
      for (int dir = 0; dir < 6; dir++) {
        if (!hasNbrAtLabel(dir)) {
          reproduceDir = dir;
          break;
        }
      }

      if (reproduceDir != -1) {
        _battery -= _demand;
        system.getCount("# Actions").record();
        system.insert(new EnergySharingParticle(
                        head.nodeInDir(localToGlobalDir(reproduceDir)), -1,
                        randDir(), system, _capacity, _demand, _transferRate,
                        _usage, State::Idle));
      }
    } else {
      Q_ASSERT(false);  // An invalid usage type was used.
    }
  }
}

int EnergySharingParticle::energyColor(int color) const {
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

EnergySharingSystem::EnergySharingSystem(int numParticles,
                                         const int numEnergyRoots,
                                         const int usage,
                                         const double capacity,
                                         const double demand,
                                         const double transferRate) {
  _counts.push_back(new Count("# Actions"));

  // Add a hexagon of idle particles to the system.
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

    insert(new EnergySharingParticle(Node(x, y), -1, randDir(), *this,
                                     capacity, demand, transferRate,
                                     static_cast<EnergySharingParticle::Usage>(usage),
                                     EnergySharingParticle::State::Idle));
  }

  // Choose particles at random to make energy ditribution roots.
  std::vector<int> indices;
  for (int i = 0; i < numParticles; ++i) {
    indices.push_back(i);
  }
  shuffle(indices.begin(), indices.end());
  for (int i = 0; i < numEnergyRoots; ++i) {
    auto ep = dynamic_cast<EnergySharingParticle*>(particles[indices[i]]);
    ep->_state = EnergySharingParticle::State::Root;
  }
}
