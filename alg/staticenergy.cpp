#include "alg/staticenergy.h"

#include <algorithm>  // for std::min, std::max.

StaticEnergyParticle::StaticEnergyParticle(const Node& head, int globalTailDir,
                                           const int orientation,
                                           AmoebotSystem& system,
                                           const double harvestRate,
                                           const double capacity,
                                           const double threshold,
                                           const double environmentEnergy,
                                           const State state)
    : AmoebotParticle (head, globalTailDir, orientation, system),
      _harvestRate(harvestRate),
      _capacity(capacity),
      _threshold(threshold),
      _environmentEnergy(environmentEnergy),
      _energyBattery(0),
      _energyBuffer(0),
      _state(state),
      _parentDir(-1) {}

void StaticEnergyParticle::activate() {
  if (_state == State::Idle) {  // Setup phase: join the spanning tree.
    if (hasNbrInState({State::Root, State::Active})) {
      _state = State::Active;
      _parentDir = hasNbrInState({State::Root}) ? labelOfFirstNbrInState({State::Root})
                                                : labelOfFirstNbrInState({State::Active});
      // Add this particle to its parent's children directions.
      for (int nbrDir = 0; nbrDir < 6; nbrDir++) {
        if (pointsAtMe(nbrAtLabel(_parentDir), nbrDir)) {
          nbrAtLabel(_parentDir)._childrenDirs.insert(nbrDir);
          break;
        }
      }
    }
  } else if (!hasNbrInState({State::Idle})) {
    // Set effective harvesting rate.
    double rate = (_energyBattery < _threshold) ? _harvestRate : 0;

    // Peripheral particles take energy from the environment; interior particles
    // take from any child with a full buffer.
    if (isOnPeriphery()) {
      _energyBattery = std::min(_energyBattery + rate * _environmentEnergy,
                                _capacity);
      _energyBuffer = std::min(_energyBuffer + (1 - rate) * _environmentEnergy,
                               _capacity);
    } else {
//      // Look for a child with a full energy buffer.
//      int fullChildDir = -1;
//      for (auto childDir : _childrenDirs) {
//        if (std::abs(nbrAtLabel(childDir)._energyBuffer - _capacity)
//            < std::numeric_limits<double>::epsilon()) {
//          fullChildDir = childDir;
//          break;
//        }
//      }

//      if (fullChildDir != -1) {
//        nbrAtLabel(fullChildDir)._energyBuffer -=
//            (std::min(rate * _capacity, _capacity - _energyBattery)
//             + std::min((1 - rate) * _capacity, _capacity - _energyBuffer));
//        _energyBattery = std::min(_energyBattery + rate * _capacity, _capacity);
//        _energyBuffer = std::min(_energyBuffer + (1 - rate) * _capacity, _capacity);
//      }

      // Look for a neighbor with a full energy buffer.
      int fullNbrDir = -1;
      for (int dir = 0; dir < 6; dir++) {
        if (hasNbrAtLabel(dir) &&
            std::abs(nbrAtLabel(dir)._energyBuffer - _capacity)
            < std::numeric_limits<double>::epsilon()) {
          fullNbrDir = dir;
          break;
        }
      }

      if (fullNbrDir != -1) {
        nbrAtLabel(fullNbrDir)._energyBuffer -=
            (std::min(rate * _capacity, _capacity - _energyBattery)
             + std::min((1 - rate) * _capacity, _capacity - _energyBuffer));
        _energyBattery = std::min(_energyBattery + rate * _capacity, _capacity);
        _energyBuffer = std::min(_energyBuffer + (1 - rate) * _capacity, _capacity);
      }
    }
  }
}

int StaticEnergyParticle::headMarkColor() const {
  if (_state == State::Root) {
    return energyColor(0xffffff, 0x000000);
  } else if (_state == State::Active) {
    return energyColor(0xffffff, 0x00ff00);
  } else {  // _state == State::Idle
    return -1;
  }
}

int StaticEnergyParticle::headMarkDir() const {
  return _parentDir;
}

int StaticEnergyParticle::tailMarkColor() const {
  return headMarkColor();
}

QString StaticEnergyParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Parameters:\n";
  text += "  harvestRate: " + QString::number(_harvestRate) + "\n";
  text += "  capacity: " + QString::number(_capacity) + "\n";
  text += "  threshold: " + QString::number(_threshold) + "\n";
  text += "  environmentEnergy: " + QString::number(_environmentEnergy) + "\n\n";
  text += "Local Info:\n";
  text += "  energyBattery: " + QString::number(_energyBattery) + " / "
                              + QString::number(_capacity) + "\n";
  text += "  energyBuffer: " + QString::number(_energyBuffer) + " / "
                             + QString::number(_capacity) + "\n";
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
  text += "  childrenDirs: [";
  for (auto childDir : _childrenDirs) {
    text += QString::number(childDir) + " ";
  }
  text += "]\n";

  return text;
}

StaticEnergyParticle& StaticEnergyParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<StaticEnergyParticle>(label);
}

int StaticEnergyParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const StaticEnergyParticle& p) {
    for (auto state : states) {
      if (p._state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<StaticEnergyParticle>(prop, startLabel);
}

bool StaticEnergyParticle::hasNbrInState(
    std::initializer_list<State> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

bool StaticEnergyParticle::isOnPeriphery() const {
  for (int dir = 0; dir < 6; dir++) {
    if (!hasNbrAtLabel(dir)) {
      return true;
    }
  }

  return false;
}

int StaticEnergyParticle::energyColor(int color1, int color2) const {
  // Parse the ints so you have color1_r, color1_g, color1_b and likewise for color2. You might need to check my math, but it should be:
  int color1_r = color1 >> 16;
  int color1_g = (color1 >> 8) % 256;
  int color1_b = color1 % 256;

  int color2_r = color2 >> 16;
  int color2_g = (color2 >> 8) % 256;
  int color2_b = color2 % 256;

  // Compute opacity.
  double opacity = (std::exp(_energyBattery - _threshold) - 1) /
                   (std::exp(_energyBattery - _threshold) + 1) + 1;
  opacity = std::max(std::min(opacity, 1.0), 0.1);

  // Compute interpolation.
  int newColor_r = color1_r + opacity * (color2_r - color1_r);
  int newColor_g = color1_g + opacity * (color2_g - color1_g);
  int newColor_b = color1_b + opacity * (color2_b - color1_b);

  // Return the int form of newColor. Again, might want to test this:
  int newColor = newColor_r;
  newColor = (newColor << 8) + newColor_g;
  newColor = (newColor << 8) + newColor_b;
  return newColor;
}

StaticEnergySystem::StaticEnergySystem(const int numParticles,
                                       const double harvestRate,
                                       const double capacity,
                                       const double threshold,
                                       const double environmentEnergy) {
  _counts.push_back(new Count("# Particles"));

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

    if (x == 0 && y == 0) {
      insert(new StaticEnergyParticle(Node(x, y), -1, randDir(), *this,
                                      harvestRate, capacity, threshold,
                                      environmentEnergy,
                                      StaticEnergyParticle::State::Root));
    } else {
      insert(new StaticEnergyParticle(Node(x, y), -1, randDir(), *this,
                                      harvestRate, capacity, threshold,
                                      environmentEnergy,
                                      StaticEnergyParticle::State::Idle));
    }
  }

  getCount("# Particles").record(numParticles);
}
