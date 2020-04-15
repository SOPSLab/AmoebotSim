#include "alg/energydistribution.h"
#include <math.h>
#include <algorithm>  // for std::min, std::max.

EnergyDistributionParticle::EnergyDistributionParticle(
    const Node& head, int globalTailDir, const int orientation,
    AmoebotSystem& system, const double harvestRate, const double inhibitedRate,
    const double capacity, const double threshold,
    const double environmentEnergy, const double GDH, const int signalSpeed,
    const State state, const int systemType)
  : AmoebotParticle (head, globalTailDir, orientation, system),
    _harvestRate(harvestRate),
    _inhibitedRate(inhibitedRate),
    _capacity(capacity),
    _threshold(threshold),
    _environmentEnergy(environmentEnergy),
    _GDH(GDH),
    _signalSpeed(signalSpeed),
    _energyBattery(0),
    _energyBuffer(0),
    _regulantBattery(0),
    _regulantBuffer(0),
    _stress(false),
    _inhibit(false),
    _signalTimer(signalSpeed),
    _state(state),
    _systemType(systemType),
    _parentDir(-1)

{
}

void EnergyDistributionParticle::activate() {
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
    communicate();
    if (_signalTimer > 0) {
      _signalTimer--;
    } else {
      _signalTimer = _signalSpeed;
      harvestEnergy();
      harvestRegulant();
      produceRegulant();
      if(_systemType){
        reproduce();
      }
    }
  }
}

int EnergyDistributionParticle::headMarkColor() const {
  int stressColor = 0xFF0000;
  int standardColor = 0x00FFFF;
  int rootColor = 0x000000;
  double glutamine = std::min(_energyBattery, _regulantBattery);
  if(_state==State::Root){
    return interpolate(0xFFFFFF,rootColor,EnergyDistributionParticle::opacity(glutamine,_threshold));
  }
  else if(_stress){
    return interpolate(0xFFFFFF,stressColor,EnergyDistributionParticle::opacity(glutamine,_threshold));
  }
  else{
    return interpolate(0xFFFFFF,standardColor,EnergyDistributionParticle::opacity(glutamine,_threshold));
  }
  return -1;
}

double EnergyDistributionParticle::opacity(double energy,double threshold) const{
    double minimum = 0.15;
    return fmax(fmin(((exp(energy-threshold)-1)/(exp(energy-threshold)+1))+1,threshold),minimum);
}

int EnergyDistributionParticle::headMarkDir() const {
  return _parentDir;
}

int EnergyDistributionParticle::tailMarkColor() const {
  return headMarkColor();
}

QString EnergyDistributionParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Parameters:\n";
  text += "  harvestRate: " + QString::number(_harvestRate) + "\n";
  text += "  inhibitedRate: " + QString::number(_inhibitedRate) + "\n";
  text += "  capacity: " + QString::number(_capacity) + "\n";
  text += "  threshold: " + QString::number(_threshold) + "\n";
  text += "  environmentEnergy: " + QString::number(_environmentEnergy) + "\n";
  text += "  GDH: " + QString::number(_GDH) + "\n";
  text += "  signalSpeed: " + QString::number(_signalSpeed) + "\n\n";
  text += "Local Info:\n";
  text += "  energyBattery: " + QString::number(_energyBattery) + " / "
                              + QString::number(_capacity) + "\n";
  text += "  energyBuffer: " + QString::number(_energyBuffer) + " / "
                             + QString::number(_capacity) + "\n";
  text += "  regulantBattery: " + QString::number(_regulantBattery) + " / "
                                + QString::number(_capacity) + "\n";
  text += "  regulantBuffer: " + QString::number(_regulantBuffer) + " / "
                               + QString::number(_capacity) + "\n";
  text += "  stress: " + QString::number(_stress) + "\n";
  text += "  inhibit: " + QString::number(_inhibit) + "\n";
  text += "  signalTimer: " + QString::number(_signalTimer) + "\n";
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

EnergyDistributionParticle& EnergyDistributionParticle::nbrAtLabel(
    int label) const {
  return AmoebotParticle::nbrAtLabel<EnergyDistributionParticle>(label);
}

int EnergyDistributionParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const EnergyDistributionParticle& p) {
    for (auto state : states) {
      if (p._state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<EnergyDistributionParticle>(prop,
                                                                 startLabel);
}

bool EnergyDistributionParticle::hasNbrInState(
    std::initializer_list<State> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

void EnergyDistributionParticle::communicate() {
  bool hasStressChild = false;
  for (auto childDir : _childrenDirs) {
    if (nbrAtLabel(childDir)._stress) {
      hasStressChild = true;
      break;
    }
  }

  if (_state != State::Root) {
    _stress = !isOnPeriphery() && ((_energyBattery < _threshold) || hasStressChild);
    _inhibit = nbrAtLabel(_parentDir)._inhibit;
  } else {
    _inhibit = (_energyBattery < _threshold) || hasStressChild;
  }
}

void EnergyDistributionParticle::harvestEnergy(){
  // Set effective harvesting rate.
  double rate;
  if (_energyBattery >= _threshold) {
    rate = 0;
  } else if (!isOnPeriphery() || (isOnPeriphery() && !_inhibit)) {
    rate = _harvestRate;
  } else {
    rate = _inhibitedRate;
  }

  // Peripheral particles take energy from the environment; interior particles
  // take from any neighbor with a full buffer.
  if (isOnPeriphery()) {
    _energyBattery = std::min(_energyBattery + rate * _environmentEnergy,
                              _capacity);
    _energyBuffer = std::min(_energyBuffer + (1 - rate) * _environmentEnergy,
                             _capacity);
  } else {
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

void EnergyDistributionParticle::harvestRegulant(){
  // Set effective harvesting rate.
  double rate;
  if (_regulantBattery >= _threshold) {
    rate = 0;
  } else if (!isOnPeriphery() || (isOnPeriphery() && !_inhibit)) {
    rate = _harvestRate;
  } else {
    rate = _inhibitedRate;
  }

  // Look for a neighbor with a full regulant buffer.
  int fullNbrDir = -1;
  for (int dir = 0; dir < 6; dir++) {
    if (hasNbrAtLabel(dir) &&
        std::abs(nbrAtLabel(dir)._regulantBuffer - _capacity)
        < std::numeric_limits<double>::epsilon()) {
      fullNbrDir = dir;
      break;
    }
  }

  // Harvest regulant from a full neighbor, if one exists.
  if (fullNbrDir != -1) {
    nbrAtLabel(fullNbrDir)._regulantBuffer -=
        (std::min(rate * _capacity, _capacity - _regulantBattery)
         + std::min((1 - rate) * _capacity, _capacity - _regulantBuffer));
    _regulantBattery = std::min(_regulantBattery + rate * _capacity, _capacity);
    _regulantBuffer = std::min(_regulantBuffer + (1 - rate) * _capacity, _capacity);
  }

  // Futile cycle causes peripheral particles to leak regulant.
  if (isOnPeriphery()) {
    _regulantBattery = std::max(_regulantBattery - _GDH, 0.0);
  }
}

void EnergyDistributionParticle::produceRegulant() {

    if ((_energyBattery >= 1) &&
      (std::min(_regulantBattery, _regulantBuffer) < _capacity)) {  // TODO: can also do <= _threshold here.
    _energyBattery -= (std::min(_harvestRate, (_capacity - _regulantBattery) / _GDH)
                       + std::min(1 - _harvestRate, (_capacity - _regulantBuffer) / _GDH));
    _regulantBattery = std::min(_regulantBattery + _harvestRate * _GDH, _capacity);
    _regulantBuffer = std::min(_regulantBuffer + (1 - _harvestRate) * _GDH, _capacity);
  }
}

void EnergyDistributionParticle::reproduce() {
  if (isOnPeriphery() && std::min(_energyBattery, _regulantBattery) >= _threshold) {
    // Pay the energy cost of reproduction.
    _energyBattery -= _threshold;
    _regulantBattery -= _threshold;

    // Find an unoccupied neighboring position.
    int reproduceDir = -1;
    for (int dir = 0; dir < 6; dir++) {
      if (!hasNbrAtLabel(dir)) {
        reproduceDir = dir;
        break;
      }
    }

    // Add a new particle to the system in the specified direction.
    system.insert(new EnergyDistributionParticle(
                    head.nodeInDir(localToGlobalDir(reproduceDir)), -1,
                    randDir(), system, _harvestRate, _inhibitedRate, _capacity,
                    _threshold, _environmentEnergy, _GDH, _signalSpeed,
                    State::Idle,_systemType));
    system.getCount("# Particles").record();
  }
}

bool EnergyDistributionParticle::isOnPeriphery() const {
  for (int dir = 0; dir < 6; dir++) {
    if (!hasNbrAtLabel(dir)) {
      return true;
    }
  }

  return false;
}

int EnergyDistributionParticle::interpolate(int color1, int color2, double intensity) const {
  // Parse the ints so you have color1_r, color1_g, color1_b and likewise for color2. You might need to check my math, but it should be:
  int color1_r = color1 >> 16;
  int color1_g = (color1 >> 8) % 256;
  int color1_b = color1 % 256;

  int color2_r = color2 >> 16;
  int color2_g = (color2 >> 8) % 256;
  int color2_b = color2 % 256;

  // Compute interpolation.
  int newColor_r = color1_r + intensity * (color2_r - color1_r);
  int newColor_g = color1_g + intensity * (color2_g - color1_g);
  int newColor_b = color1_b + intensity * (color2_b - color1_b);

  // Return the int form of newColor. Again, might want to test this:
  int newColor = newColor_r;
  newColor = (newColor << 8) + newColor_g;
  newColor = (newColor << 8) + newColor_b;
  return newColor;
}

EnergyDistributionSystem::EnergyDistributionSystem(
    const int numParticles, const int systemType, const double harvestRate,
        const double inhibitedRate, const double capacity, const double threshold,
        const double environmentEnergy,const double GDH,const int signalSpeed) {


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
    if(x==0&&y==0){
        insert(new EnergyDistributionParticle(
                 Node(x,y), -1, randDir(), *this, harvestRate,
                 inhibitedRate, capacity, threshold, environmentEnergy, GDH,
                 signalSpeed, EnergyDistributionParticle::State::Root,systemType));
    }
    else{
        insert(new EnergyDistributionParticle(
                 Node(x,y), -1, randDir(), *this, harvestRate,
                 inhibitedRate, capacity, threshold, environmentEnergy, GDH,
                 signalSpeed, EnergyDistributionParticle::State::Idle,systemType));
    }
  }
  getCount("# Particles").record(numParticles);
}
