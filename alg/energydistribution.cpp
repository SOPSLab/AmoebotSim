#include "alg/energydistribution.h"

#include <algorithm>  // for std::min, std::max.

EnergyDistributionParticle::EnergyDistributionParticle(
    const Node& head, int globalTailDir, const int orientation,
    AmoebotSystem& system, const double harvestRate, const double inhibitedRate,
    const double capacity, const double threshold,
    const double environmentEnergy, const double GDH, const int signalSpeed,
    const State state)
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
    _parentDir(-1) {
  _colorState = -1;  // TODO: remove this.
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
      reproduce();
    }
  }
}

int EnergyDistributionParticle::headMarkColor() const {
  int color = 0x00FF00;
  int color2 = 0x00FFFF;
  int color3 = 0xFF0000;

  switch(_colorState) {
    case 0:  return 800080;
    case 1:  return interpolate(0xFFFFFF,color,1.0);
    case 2:  return interpolate(0xFFFFFF,color,0.9);
    case 3:  return interpolate(0xFFFFFF,color,0.8);
    case 4:  return interpolate(0xFFFFFF,color,0.7);
    case 5:  return interpolate(0xFFFFFF,color,0.6);
    case 6:  return interpolate(0xFFFFFF,color,0.5);
    case 7:  return interpolate(0xFFFFFF,color,0.4);
    case 8:  return interpolate(0xFFFFFF,color,0.3);
    case 9:  return interpolate(0xFFFFFF,color,0.27);
    case 10: return interpolate(0xFFFFFF,color,0.25);
    case 11: return interpolate(0xFFFFFF,color,0.24);
    case 12: return interpolate(0xFFFFFF,color,0.22);
    case 13: return interpolate(0xFFFFFF,color,0.2);
    case 14: return interpolate(0xFFFFFF,color,0.15);

    case 15: return interpolate(0xFFFFFF,color2,1.0);
    case 16: return interpolate(0xFFFFFF,color2,0.9);
    case 17: return interpolate(0xFFFFFF,color2,0.8);
    case 18: return interpolate(0xFFFFFF,color2,0.7);
    case 19: return interpolate(0xFFFFFF,color2,0.6);
    case 20: return interpolate(0xFFFFFF,color2,0.5);
    case 21: return interpolate(0xFFFFFF,color2,0.4);
    case 22: return interpolate(0xFFFFFF,color2,0.3);
    case 23: return interpolate(0xFFFFFF,color2,0.27);
    case 24: return interpolate(0xFFFFFF,color2,0.25);
    case 25: return interpolate(0xFFFFFF,color2,0.24);
    case 26: return interpolate(0xFFFFFF,color2,0.22);
    case 27: return interpolate(0xFFFFFF,color2,0.2);
    case 28: return interpolate(0xFFFFFF,color2,0.15);

    case 29: return interpolate(0xFFFFFF,color3,1.0);
    case 30: return interpolate(0xFFFFFF,color3,0.9);
    case 31: return interpolate(0xFFFFFF,color3,0.8);
    case 32: return interpolate(0xFFFFFF,color3,0.7);
    case 33: return interpolate(0xFFFFFF,color3,0.6);
    case 34: return interpolate(0xFFFFFF,color3,0.5);
    case 35: return interpolate(0xFFFFFF,color3,0.4);
    case 36: return interpolate(0xFFFFFF,color3,0.3);
    case 37: return interpolate(0xFFFFFF,color3,0.27);
    case 38: return interpolate(0xFFFFFF,color3,0.25);
    case 39: return interpolate(0xFFFFFF,color3,0.24);
    case 40: return interpolate(0xFFFFFF,color3,0.22);
    case 41: return interpolate(0xFFFFFF,color3,0.2);
    case 42: return interpolate(0xFFFFFF,color3,0.15);
  }

  return -1;
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
    _stress = (_energyBattery < _threshold) || hasStressChild;
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
    updateState();  // TODO: remove later.
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

      // TODO: remove later.
      nbrAtLabel(fullNbrDir).updateState();
      updateState();
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

    // TODO: remove later.
    nbrAtLabel(fullNbrDir).updateState();
    updateState();
  }

  // Futile cycle causes peripheral particles to leak regulant.
  if (isOnPeriphery()) {
    _regulantBattery = std::max(_regulantBattery - _GDH, 0.0);
    updateState();  // TODO: remove later.
  }
}

void EnergyDistributionParticle::produceRegulant() {
  if ((_energyBattery >= 1) &&
      (std::min(_regulantBattery, _regulantBuffer) < _capacity)) {  // TODO: can also do <= _threshold here.
    _energyBattery -= (std::min(_harvestRate, (_capacity - _regulantBattery) / _GDH)
                       + std::min(1 - _harvestRate, (_capacity - _regulantBuffer) / _GDH));
    _regulantBattery = std::min(_regulantBattery + _harvestRate * _GDH, _capacity);
    _regulantBuffer = std::min(_regulantBuffer + (1 - _harvestRate) * _GDH, _capacity);
    updateState();  // TODO: Remove later
  }
}

void EnergyDistributionParticle::reproduce() {
  if (isOnPeriphery() && std::min(_energyBattery, _regulantBattery) >= _threshold) {
    // Pay the energy cost of reproduction.
    _energyBattery -= _threshold;
    _regulantBattery -= _threshold;
    updateState();  // TODO: remove.

    // Find an unoccupied neighboring position.
    int reproduceDir = -1;
    for (int dir = 0; dir < 6; dir++) {
      if (!hasNbrAtLabel(dir)) {
        reproduceDir = dir;
        break;
      }
    }
    Q_ASSERT(reproduceDir != -1);

    // Add a new particle to the system in the specified direction.
    system.insert(new EnergyDistributionParticle(
                          head.nodeInDir(reproduceDir), -1, randDir(), system,
                          _harvestRate, _inhibitedRate, _capacity, _threshold,
                          _environmentEnergy, _GDH, _signalSpeed, State::Idle));

    // Set this particle as the new particle's parent in the spanning tree.
    for (int nbrDir = 0; nbrDir < 0; nbrDir++) {
      if (pointsAtMe(nbrAtLabel(reproduceDir), nbrDir)) {
        nbrAtLabel(reproduceDir)._parentDir = nbrDir;
        break;
      }
    }
    _childrenDirs.insert(reproduceDir);
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

void EnergyDistributionParticle::updateState() {
  double bracket = _threshold / 14.0;
  double glutamine = std::min(_energyBattery, _regulantBattery);
  if (_parentDir == -1) {
    if (glutamine < bracket) {
        _colorState = 42;
    } else if (glutamine < 2 * bracket) {
        _colorState = 41;
    } else if (glutamine < 3 * bracket) {
        _colorState = 40;
    } else if (glutamine < 4 * bracket) {
        _colorState = 39;
    }
    else if(glutamine < 5*bracket){
        _colorState = 38;
    }
    else if(glutamine < 6*bracket){
        _colorState =  37;
    }
    else if(glutamine < 7*bracket){
        _colorState = 36;
    }
    else if(glutamine < 8*bracket){
       _colorState = 35;
    }
    else if(glutamine < 9*bracket){
        _colorState = 34;
    }
    else if(glutamine < 10*bracket){
        _colorState = 33;
    }
    else if(glutamine < 11*bracket){
        _colorState = 32;
    }
    else if(glutamine < 12*bracket){
        _colorState = 31;
    }
    else if(glutamine < 13*bracket){
        _colorState = 30;
    }
    else if(glutamine <= _capacity){
        _colorState = 29;
    }
  }
  else if(!isOnPeriphery()){
      if(glutamine < bracket){
          _colorState = 14;
      }
      else if(glutamine < 2*bracket){
          _colorState = 13;
      }
      else if(glutamine < 3*bracket){
          _colorState = 12;
      }
      else if(glutamine < 4*bracket){
          _colorState = 11;
      }
      else if(glutamine < 5*bracket){
          _colorState = 10;
      }
      else if(glutamine < 6*bracket){
          _colorState =  9;
      }
      else if(glutamine < 7*bracket){
          _colorState = 8;
      }
      else if(glutamine < 8*bracket){
         _colorState = 7;
      }
      else if(glutamine < 9*bracket){
          _colorState = 6;
      }
      else if(glutamine < 10*bracket){
          _colorState = 5;
      }
      else if(glutamine < 11*bracket){
          _colorState = 4;
      }
      else if(glutamine < 12*bracket){
          _colorState = 3;
      }
      else if(glutamine < 13*bracket){
          _colorState = 2;
      }
      else if(glutamine <= _capacity){
          _colorState = 1;
      }
  }
  else{
      if(glutamine < bracket){
          _colorState = 28;
      }
      else if(glutamine < 2*bracket){
          _colorState = 27;
      }
      else if(glutamine < 3*bracket){
          _colorState = 26;
      }
      else if(glutamine < 4*bracket){
          _colorState = 25;
      }
      else if(glutamine < 5*bracket){
          _colorState = 24;
      }
      else if(glutamine < 6*bracket){
          _colorState =  23;
      }
      else if(glutamine < 7*bracket){
          _colorState = 22;
      }
      else if(glutamine < 8*bracket){
         _colorState = 21;
      }
      else if(glutamine < 9*bracket){
          _colorState = 20;
      }
      else if(glutamine < 10*bracket){
          _colorState = 19;
      }
      else if(glutamine < 11*bracket){
          _colorState = 18;
      }
      else if(glutamine < 12*bracket){
          _colorState = 17;
      }
      else if(glutamine < 13*bracket){
          _colorState = 16;
      }
      else if(glutamine <= _capacity){
          _colorState = 15;
      }
  }
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
    const double harvestRate, const double inhibitedRate, const double capacity,
    const double threshold, const double environmentEnergy, const double GDH,
    const int signalSpeed) {
  // Initialize the root particle.
  Node origin(0, 0);
  insert(new EnergyDistributionParticle(
           origin, -1, randDir(), *this, harvestRate, inhibitedRate, capacity,
           threshold, environmentEnergy, GDH, signalSpeed,
           EnergyDistributionParticle::State::Root));

  // Initialize the surrounding particles.
  for (int dir = 0; dir < 6; dir++) {
    insert(new EnergyDistributionParticle(
             origin.nodeInDir(dir), -1, randDir(), *this, harvestRate,
             inhibitedRate, capacity, threshold, environmentEnergy, GDH,
             signalSpeed, EnergyDistributionParticle::State::Idle));
  }
}

/* OLD CODE */

/*
void EnergyParticle::activate(){
    //Check signal
    if(_signalTimer == 0){
        _signalTimer = _signalSpeed;
        //We want stressed particles in interior to always be able to eat
        if(_stressRoot){
            _canEat = true;
        }
        if(_canEat){
           _effectiveRate = _consumptionRate;
        }
        else{
            _effectiveRate = _restrictedRate*_inhibitedReuptakeRate;
        }
        if(_consumedGlutamate >= _hungerThreshold){
            _effectiveRate = 0.0;
        }
        //Consume Glutamate
        if(hasEmptyNeighbor() != -1){
            _consumedGlutamate = fmin(_consumedGlutamate + (_environmentalGlutamate*_effectiveRate),_energyStorageCap);
            _remainingGlutamate = fmin(_remainingGlutamate + (_environmentalGlutamate*(1-_effectiveRate)),_energyStorageCap);
            updateState();
        }
        else{
            EnergyParticle* m = argMax(1);
            if(m != nullptr){
                if(m->_remainingGlutamate==_energyStorageCap){
                    double temp =_consumedGlutamate;
                    _consumedGlutamate = fmin(_consumedGlutamate + (m->_remainingGlutamate * _effectiveRate),_energyStorageCap);
                    double delta = _consumedGlutamate-temp;
                    m->_remainingGlutamate -= delta;
                    temp = _remainingGlutamate;
                    _remainingGlutamate = fmin(_remainingGlutamate + (m->_remainingGlutamate * (1-_effectiveRate)),_energyStorageCap);
                    delta = _remainingGlutamate-temp;
                    m->_remainingGlutamate -= delta;
                    m->updateState();
                    updateState();
                }
            }
        }
        if(_canEat){
           _effectiveRate = _consumptionRate;
        }
        else{
            _effectiveRate = _restrictedRate;
        }
        if(_consumedAmmonium >= _hungerThreshold){
            _effectiveRate = 0.0;
        }
        //Consume Ammonium
        EnergyParticle* m = argMax(0);
        if(m != nullptr){
            if(m->_remainingAmmonium==_energyStorageCap){
                double temp = _consumedAmmonium;
                _consumedAmmonium = fmin(_consumedAmmonium+(m->_remainingAmmonium * _effectiveRate),_energyStorageCap);
                double delta = _consumedAmmonium-temp;
                m->_remainingAmmonium -= delta;
                temp = _remainingAmmonium;
                _remainingAmmonium = fmin(_remainingAmmonium + (m->_remainingAmmonium * (1-_effectiveRate)),_energyStorageCap);
                delta = _remainingAmmonium-temp;
                m->_remainingAmmonium -= delta;
                m->updateState();
                updateState();
            }
        }
        //Production Routine
        //Reproduce
        int emptyNeighborDir = hasEmptyNeighbor();
        if(_inTelophase){
            if(doGlobalAnalysis){
                globalInteriorEnergy();
            }
            _prevX = this->head.x;
            _prevY = this->head.y;
            contractHead();
            for(int i = 0; i < 6; ++i){
                if (_children[i] == nullptr){
                    _children[i] = &mitosis(_prevX,_prevY);
                    _children[i]->_parent = this;
                    break;
                }
            }
            _inTelophase = false;
            _prevX = _prevY = -1;
        }
        else if(emptyNeighborDir != -1 && _consumedGlutamate >= _hungerThreshold && _consumedAmmonium >= _hungerThreshold){
            expand(emptyNeighborDir);
            _consumedGlutamate = _consumedGlutamate - _hungerThreshold;
            _consumedAmmonium = _consumedAmmonium - _hungerThreshold;
            _inTelophase = true;
        }
        //Produce Ammonium
        if(_consumedGlutamate >= _glutamateCost && hasEmptyNeighbor() == -1 && (_remainingAmmonium < _energyStorageCap || _consumedAmmonium < _energyStorageCap)){
            _consumedAmmonium = fmin(_consumedAmmonium+(_consumptionRate*_ammoniumBenefit),_energyStorageCap);
            _remainingAmmonium = fmin(_remainingAmmonium+((1.0-_consumptionRate)*_ammoniumBenefit),_energyStorageCap);
            _consumedGlutamate-=_glutamateCost;
            updateState();
        }
        else if(hasEmptyNeighbor() != -1 && _consumedGlutamate >= _glutamateCost){
            _consumedGlutamate -= _glutamateCost;
        }
        //Cost of Living
        //_consumedGlutamate -= 0.5;
        //_consumedAmmonium -= 0.5;
    }
    else{
        _signalTimer--;
    }

    //Message Passing via ion channels
    //Hunger triggers root stress
    if(hasEmptyNeighbor()==-1 && _consumedGlutamate<_hungerThreshold){
        _stressRoot = true;
    }
    //Propagate stress signal to root
    if(_stressRoot){
        if(_parent != nullptr){
            _parent->_stressRoot = true;
        }
        else{
            _stressed = true;
        }
    }
    //System Stress originate from the root. Root checks if any of its neighbors ancestors are stressed. If not, it becomes unstressed, freeing the system,
    //Stress signal only goes one way. (Toward chilrdren)
    if(_parent == nullptr){
        if(_stressRoot){
            bool stressCheck = false;
            for(int i = 0; i < 6; ++i){
                if(_children[i] != nullptr){
                    if(_children[i]->_stressRoot && _children[i]->hasEmptyNeighbor()==-1){
                        stressCheck = true;
                    }
                }
            }
            if(!stressCheck){
                _stressed = false;
            }
        }
    }
    //No longer hungry, stop stressing root
    if(_stressRoot && _consumedGlutamate >= _hungerThreshold){
        _stressRoot = false;
    }

    //Propagate either halting signal or resuming signal
    if(_stressed){
        for(int i = 0; i< 6; ++i){
            if(_children[i]!=nullptr){
                _children[i]->_stressed = true;
                if(_children[i]->hasEmptyNeighbor()!=-1){
                    _children[i]->_canEat = false;
                }
            }
        }
    }
    else{
        for(int i = 0; i< 6; ++i){
            if(_children[i]!=nullptr){
                _children[i]->_stressed = false;
                _children[i]->_canEat = true;
            }
        }
    }


    //Gather global data
    if(doGlobalAnalysis){
       // globalMetrics();
    }
}

EnergyDistributionSystem::EnergyDistributionSystem(double consumptionRate, double restrictedRate,
                                     double hungerThreshold, double energyStorageCap,
                                     double environmentalGlutamate, double glutamateCost, double ammoniumBenefit, double inhibitedReuptakeRate, int signalSpeed) {
  list<EnergyDistributionParticle*> allParticles;
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
      if i = 1 put it in as root, else put in as idle
      insert(new CompressionParticle(Node(x, y), -1, randDir(), *this, lambda));

    //Place Root on periphery
    allParticles.back()->_parent = nullptr;
    allParticles.back()->_children[0] = root;
    root->_parent = allParticles.back();
    root->_children[6]=nullptr;

    //Initialize outer 12 particles
    for(int k = 0; k < 6; ++k){
        for(int i = 0; i<6;++i){
            for(int j=0; j < 6; ++j){
                int expandDir = randDir();
                if(!particles[k]->hasNbrAtLabel(expandDir)){
                    particles[k]->expandInit(expandDir);
                    int x = particles[k]->head.x;
                    int y = particles[k]->head.y;
                    particles[k]->contractInit();
                    EnergyParticle* p = &particles[k]->mitosis(x,y);
                    p->_parent = particles[k];
                    particles[k]->_children[i] = p;
                    break;
                }
            }
        }
    }
//    numTotalParticles = numPrevParticles = 7;
    //this->_numMovements = 0;
    //file.open ("special_data.txt");
}
*/
