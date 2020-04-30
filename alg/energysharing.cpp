#include "alg/energysharing.h"

#include <algorithm>  // for std::min, std::max.

EnergySharingParticle::EnergySharingParticle(const Node& head,
                                             int globalTailDir,
                                             const int orientation,
                                             AmoebotSystem& system,
                                             const double capacity,
                                             const double harvestRate,
                                             const double batteryFrac,
                                             const Usage usage,
                                             const State state)
    : AmoebotParticle (head, globalTailDir, orientation, system),
      _capacity(capacity),
      _harvestRate(harvestRate),
      _batteryFrac(batteryFrac),
      _demand(0.25 * _capacity),
      _usage(usage),
      _battery(0),
      _buffer(0),
      _stress(false),
      _inhibit(false),
      _state(state),
      _parentDir(-1) {}

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

    int nbrDir = labelOfFirstNbrWithProperty<EnergySharingParticle>(prop);
    if (nbrDir != -1) {
      _state = State::Active;
      _parentDir = nbrDir;
    }
  } else {
    communicate();
    harvestEnergy();
    useEnergy();
  }
}

int EnergySharingParticle::headMarkColor() const {
  if (_state == State::Root) {
    return energyColor(0xffffff, 0x000000);
  } else if (_stress) {
    return energyColor(0xffffff, 0xff0000);
  } else if (_inhibit) {
    return energyColor(0xffffff, 0xfcd703);
  } else if (_state == State::Active) {
    return energyColor(0xffffff, 0x00ff00);
  } else {  // _state == State::Idle
    return -1;
  }
}

int EnergySharingParticle::headMarkDir() const {
  return _parentDir;
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
  text += "  battery: " + QString::number(_battery) + " / "
                        + QString::number(_capacity) + "\n";
  text += "  buffer: " + QString::number(_buffer) + " / "
                       + QString::number(_capacity) + "\n";
  text += "  stress: " + QString::number(_stress) + "\n";
  text += "  inhibit: " + QString::number(_inhibit) + "\n";
  text += "  state: ";
  text += [this](){
    switch(_state) {
      case State::Root:   return "root\n";
      case State::Idle:   return "idle\n";
      case State::Active: return "active\n";
    }
    return "no state\n";
  }();
  text += "  parentDir: " + QString::number(_parentDir) + "\n";

  return text;
}

EnergySharingParticle& EnergySharingParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<EnergySharingParticle>(label);
}

void EnergySharingParticle::communicate() {
  bool hasStressChild = false;
  for (int nbrDir = 0; nbrDir < 6; nbrDir++) {
    if (hasNbrAtLabel(nbrDir) && nbrAtLabel(nbrDir)._stress
        && pointsAtMe(nbrAtLabel(nbrDir), nbrAtLabel(nbrDir)._parentDir)) {
      hasStressChild = true;
      break;
    }
  }

  if (_state != State::Root) {
    _stress = _battery < _demand || hasStressChild;
    _inhibit = nbrAtLabel(_parentDir)._inhibit;
  } else {
    _inhibit = _battery < _demand || hasStressChild;
  }
}

void EnergySharingParticle::harvestEnergy() {
  // Set effective battery fraction.
  double frac = (_battery < _capacity) ? _batteryFrac : 0;

  // Particles with energy access take from the source; others take from their
  // parents if they have a full buffer.
  if (_state == State::Root) {
    _battery = std::min(_battery + frac * _harvestRate, _capacity);
    _buffer = std::min(_buffer + (1 - frac) * _harvestRate, _capacity);
  } else if (nbrAtLabel(_parentDir)._buffer >= _harvestRate) {
    nbrAtLabel(_parentDir)._buffer -=
        (std::min(frac * _harvestRate, _capacity - _battery)
         + std::min((1 - frac) * _harvestRate, _capacity - _buffer));
    _battery = std::min(_battery + frac * _harvestRate, _capacity);
    _buffer = std::min(_buffer + (1 - frac) * _harvestRate, _capacity);
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
                        randDir(), system, _capacity, _harvestRate,
                        _batteryFrac, _usage, State::Idle));
      }
    } else {
      Q_ASSERT(false);  // An invalid usage type was used.
    }
  }
}

int EnergySharingParticle::energyColor(int color1, int color2) const {
  // Parse the color ints into RGB values.
  int color1_r = color1 >> 16;
  int color1_g = (color1 >> 8) % 256;
  int color1_b = color1 % 256;

  int color2_r = color2 >> 16;
  int color2_g = (color2 >> 8) % 256;
  int color2_b = color2 % 256;

  // Compute opacity.
  double opacity = (std::exp(_battery - _demand) - 1) /
                   (std::exp(_battery - _demand) + 1) + 1;
  opacity = std::max(std::min(opacity, 1.0), 0.05);

  // Compute interpolation.
  int newColor_r = color1_r + opacity * (color2_r - color1_r);
  int newColor_g = color1_g + opacity * (color2_g - color1_g);
  int newColor_b = color1_b + opacity * (color2_b - color1_b);

  // Return the int form of newColor.
  int newColor = newColor_r;
  newColor = (newColor << 8) + newColor_g;
  newColor = (newColor << 8) + newColor_b;

  return newColor;
}

EnergySharingSystem::EnergySharingSystem(int numParticles, const int usage,
                                         const double capacity,
                                         const double harvestRate,
                                         const double batteryFrac) {
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
                                     capacity, harvestRate, batteryFrac,
                                     static_cast<EnergySharingParticle::Usage>(usage),
                                     EnergySharingParticle::State::Idle));
  }

  // Mark only the first particle as a root with energy access.
  auto p = particleMap.find(Node(0, 0))->second;
  auto ep = dynamic_cast<EnergySharingParticle*>(p);
  ep->_state = EnergySharingParticle::State::Root;

//  // Mark the particles with energy access as roots.
//  for (auto p : particles) {
//    auto ep = dynamic_cast<EnergySharingParticle*>(p);
//    for (int dir = 0; dir < 6; dir++) {
//      if (!ep->hasNbrAtLabel(dir)) {
//        ep->_state = EnergySharingParticle::State::Root;
//        break;
//      }
//    }
//  }
}
