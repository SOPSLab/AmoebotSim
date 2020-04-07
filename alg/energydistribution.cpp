#include "alg/energydistribution.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <iterator>

using namespace std;

//Global measurement variables (not used in algorithm)
int numTotalParticles = 0;
int numPrevParticles = numTotalParticles;
int roundNum = 0;
const int maxRounds = 15000;
const int measurementRoundFrequency = 50;
const bool doGlobalAnalysis = false;
const bool doAggregateAnalysis  = false;
bool roundFlag = false;
bool roundFlag2 = false;
double totalPercentage;
int numExpansions;
list<EnergyParticle*> allParticles;
ofstream file;

//Particle constructor
EnergyParticle::EnergyParticle(const Node head, const int globalTailDir, const int orientation, AmoebotSystem& system,
                               double consumptionRate, double restrictedRate,
                               double hungerThreshold, double energyStorageCap,
                               double environmentalGlutamate, double glutamateCost,double ammoniumBenefit, double inhibitedReuptakeRate, int signalSpeed):AmoebotParticle (head,globalTailDir,orientation,system){
    _treeState = idle;
    _state = -1;
    _sys = &system;
    _energyBattery = 0.0;
    _energyBuffer  = 0.0;
    _regulantBattery = 0.0;
    _regulantBuffer = 0.0;
    _stress = false;
    _inhibit = false;


    //inherited local variables
    _consumptionRate = consumptionRate;
    _hungerThreshold = hungerThreshold;
    _energyStorageCap = energyStorageCap;
    _environmentalGlutamate = environmentalGlutamate;
    _regulantConversion= glutamateCost;
    _GDH = ammoniumBenefit;
    _inTelophase = false;
    _expandDir = -1;
    _prevX = -1;
    _prevY = -1;
    _children = new EnergyParticle*[6];
    _signalSpeed = signalSpeed;
    _signalTimer = _signalSpeed;
    for(int i = 0; i <6; ++i){
        _children[i] = nullptr;
    }
    _parent = nullptr;
}
//Activation function
void EnergyParticle::activate(){
    //Algorithm 1

    //Attach Self to spanning tree
    //Search Neighbor states
    EnergyParticle* rootNeighbor = nullptr;
    EnergyParticle* followerNeighbor = nullptr;
    EnergyParticle* idleNeighbor =nullptr;
    for(int i=0;i<6;++i){
        if(hasNbrAtLabel(i)){
            if(nbrAtLabel(i)._treeState==root){
                rootNeighbor = &nbrAtLabel(i);
            }
            if(nbrAtLabel(i)._treeState==follower){
                followerNeighbor = &nbrAtLabel(i);
            }
            if(nbrAtLabel(i)._treeState==idle){
                idleNeighbor = &nbrAtLabel(i);
            }
        }
    }
    //Set Parent
    if(_treeState == idle){
        if(rootNeighbor != nullptr){
            _parent = rootNeighbor;
            for(int i = 0;i<6;++i){
                if(_parent->_children[i] == nullptr){
                    _parent->_children[i]= this;
                }
            }
        }
        else if(followerNeighbor != nullptr){
            _parent = followerNeighbor;
            for(int i = 0;i<6;++i){
                if(_parent->_children[i] == nullptr){
                    _parent->_children[i]= this;
                }
            }
        }

    }
    else if(idleNeighbor == nullptr){
        communicate();
        if(_signalTimer>0){
            _signalTimer--;
        }
        else{
            _signalTimer = _signalSpeed;
            harvestEnergy();
            harvestRegulant();
            produceRegulant();
            reproduce();
        }
    }

}

void EnergyParticle::communicate(){
    //algorithm 2

    //Scan for stressed child
    bool hasStressChild = false;
    for(int i=0; i<6;++i){
        if(_children[i] != nullptr){
            if(_children[i]->_stress){
                hasStressChild = true;
            }
        }
    }
    if(_treeState != root){
        if(_energyBattery<_hungerThreshold || hasStressChild){
            _stress = true;
        }
        else{
            _stress = false;
        }
        _inhibit=_parent->_inhibit;
    }
    else{
        if(_energyBattery<_hungerThreshold || hasStressChild){
           _inhibit = true;
        }
        else{
            _inhibit = false;
        }
    }
}

void EnergyParticle::harvestEnergy(){

    //Determine whether on periphery or interior
    bool onPeriphery = hasEmptyNeighbor();
    double rate;
    if(_energyBattery >= _hungerThreshold){
        rate = 0.0;
    }
    else if(!onPeriphery || (onPeriphery && !_inhibit)){
        rate = _consumptionRate;
    }
    else{
        rate = _restrictedRate;
    }
    if(onPeriphery){
        _energyBattery=fmin(_energyBattery+rate*_environmentalGlutamate,_energyStorageCap);
        _energyBuffer=fmin(_energyBuffer+(1-rate)*_environmentalGlutamate,_energyStorageCap);
        updateState();
    }
    else{
        EnergyParticle* Q = argMax(1);
        if(Q != nullptr){
            if(Q->_energyBuffer==_energyStorageCap){
                Q->_energyBuffer=Q->_energyBuffer-fmin(rate*_energyStorageCap,_energyStorageCap-_energyBattery)-fmin((1-rate)*_energyStorageCap,_energyStorageCap-_energyBuffer);
                _energyBattery=fmin(_energyBattery+rate*_energyStorageCap,_energyStorageCap);
                _energyBuffer=fmin(_energyBuffer+(1-rate)*_energyStorageCap,_energyStorageCap);
                Q->updateState();
                updateState();
            }
        }
    }

}

void EnergyParticle::harvestRegulant(){
    //Determine whether on periphery or interior
    bool onPeriphery = hasEmptyNeighbor();
    double rate;
    if(_regulantBattery >= _hungerThreshold){
        rate = 0.0;
    }
    else if(!onPeriphery || (onPeriphery && !_inhibit)){
        rate = _consumptionRate;
    }
    else{
        rate = _restrictedRate;
    }
    EnergyParticle* Q = argMax(0);
    if(Q != nullptr){
        if(Q->_regulantBuffer==_energyStorageCap){
            Q->_regulantBuffer=Q->_regulantBuffer-fmin(rate*_energyStorageCap,_energyStorageCap-_regulantBattery)-fmin((1-rate)*_energyStorageCap,_energyStorageCap-_regulantBuffer);
            _regulantBattery=fmin(_regulantBattery+rate*_energyStorageCap,_energyStorageCap);
            _regulantBuffer=fmin(_regulantBuffer+(1-rate)*_energyStorageCap,_energyStorageCap);
            Q->updateState();
            updateState();
        }
    }
    //Futile Cycle
    if(onPeriphery){
        _regulantBattery--;
        updateState();
    }

}

void EnergyParticle::produceRegulant(){
    if(_energyBattery>=1 && fmin(_regulantBuffer,_regulantBattery)<=_hungerThreshold){
        _energyBattery=_energyBattery-fmin(_consumptionRate,(_energyStorageCap-_regulantBattery)/_GDH)-fmin(1-_consumptionRate,(_energyStorageCap-_regulantBuffer)/_GDH);
        _regulantBattery = fmin(_regulantBattery+_consumptionRate*_GDH,_energyStorageCap);
        _regulantBuffer = fmin(_regulantBuffer+(1-_consumptionRate)*_GDH,_energyStorageCap);
        updateState();
    }
}

void EnergyParticle::reproduce(){
    //Reproduce
    int emptyNeighborDir = hasEmptyNeighborInDir();
    if(_inTelophase){
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
    else if(emptyNeighborDir != -1 && _energyBattery >= _hungerThreshold && _regulantBattery >= _hungerThreshold){
        expand(emptyNeighborDir);
        _energyBattery = _energyBattery - _hungerThreshold;
        _regulantBattery = _regulantBattery - _hungerThreshold;
        _inTelophase = true;
    }
}

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
*/
//Activation helper functions

//Returns a reference to a neighboring particle with the largest amount of a certain energy type
EnergyParticle* EnergyParticle::argMax(int type){
    //Type 1 gives argmax for energy, Type 0 gives argmax for regulant
    EnergyParticle* maxP = nullptr;
    double maxVal = -1.0;
    if(type == 1){
        for(int i = 0; i < 6; ++i){
            if(hasNbrAtLabel(i)){
                if(nbrAtLabel(i)._energyBuffer >= maxVal){
                    maxP = &nbrAtLabel(i);
                    maxVal = nbrAtLabel(i)._energyBuffer;
                }
            }
        }
    }
    if(type == 0){
        for(int i = 0; i < 6; ++i){
            if(hasNbrAtLabel(i)){
                if(nbrAtLabel(i)._regulantBuffer >= maxVal){
                    maxP = &nbrAtLabel(i);
                    maxVal = nbrAtLabel(i)._regulantBuffer;
                }
            }
        }
    }
    return maxP;
}

bool EnergyParticle::hasEmptyNeighbor(){
    for(int i = 0; i < 6; ++i){
        if(!hasNbrAtLabel(i)){
            return true;
        }
    }
    return false;
}

int EnergyParticle::hasEmptyNeighborInDir(){
    for(int i = 0; i < 6; ++i){
        if(!hasNbrAtLabel(i)){
            return i;
        }
    }
    return -1;
}

//Function allows a particle to reproduce into an empty neighboring location
EnergyParticle& EnergyParticle::mitosis(int x, int y){
        Node node(x,y);
        EnergyParticle* particle = new EnergyParticle(node,-1,randDir(),*_sys,_consumptionRate,_restrictedRate,
                                                      _hungerThreshold,_energyStorageCap,_environmentalGlutamate,_glutamateCost,_ammoniumBenefit,_inhibitedReuptakeRate,_signalSpeed);
        _sys->EnergyDistributionSystem::insert(particle);
        if(particle != nullptr){
            numTotalParticles++;
        }
        allParticles.push_back(particle);
        return *particle;
}

//Functions related to particle color

//Changes particle state
void EnergyParticle::updateState(){
    double bracket = _hungerThreshold / 14.0;
    double glutamine = fmin(_energyBattery,_regulantBattery);
    if(_parent==nullptr){
        if(glutamine < bracket){
            _state = 42;
        }
        else if(glutamine < 2*bracket){
            _state = 41;
        }
        else if(glutamine < 3*bracket){
            _state = 40;
        }
        else if(glutamine < 4*bracket){
            _state = 39;
        }
        else if(glutamine < 5*bracket){
            _state = 38;
        }
        else if(glutamine < 6*bracket){
            _state =  37;
        }
        else if(glutamine < 7*bracket){
            _state = 36;
        }
        else if(glutamine < 8*bracket){
           _state = 35;
        }
        else if(glutamine < 9*bracket){
            _state = 34;
        }
        else if(glutamine < 10*bracket){
            _state = 33;
        }
        else if(glutamine < 11*bracket){
            _state = 32;
        }
        else if(glutamine < 12*bracket){
            _state = 31;
        }
        else if(glutamine < 13*bracket){
            _state = 30;
        }
        else if(glutamine <= _energyStorageCap){
            _state = 29;
        }
    }
    else if(!hasEmptyNeighbor()){
        if(glutamine < bracket){
            _state = 14;
        }
        else if(glutamine < 2*bracket){
            _state = 13;
        }
        else if(glutamine < 3*bracket){
            _state = 12;
        }
        else if(glutamine < 4*bracket){
            _state = 11;
        }
        else if(glutamine < 5*bracket){
            _state = 10;
        }
        else if(glutamine < 6*bracket){
            _state =  9;
        }
        else if(glutamine < 7*bracket){
            _state = 8;
        }
        else if(glutamine < 8*bracket){
           _state = 7;
        }
        else if(glutamine < 9*bracket){
            _state = 6;
        }
        else if(glutamine < 10*bracket){
            _state = 5;
        }
        else if(glutamine < 11*bracket){
            _state = 4;
        }
        else if(glutamine < 12*bracket){
            _state = 3;
        }
        else if(glutamine < 13*bracket){
            _state = 2;
        }
        else if(glutamine <= _energyStorageCap){
            _state = 1;
        }
    }
    else{
        if(glutamine < bracket){
            _state = 28;
        }
        else if(glutamine < 2*bracket){
            _state = 27;
        }
        else if(glutamine < 3*bracket){
            _state = 26;
        }
        else if(glutamine < 4*bracket){
            _state = 25;
        }
        else if(glutamine < 5*bracket){
            _state = 24;
        }
        else if(glutamine < 6*bracket){
            _state =  23;
        }
        else if(glutamine < 7*bracket){
            _state = 22;
        }
        else if(glutamine < 8*bracket){
           _state = 21;
        }
        else if(glutamine < 9*bracket){
            _state = 20;
        }
        else if(glutamine < 10*bracket){
            _state = 19;
        }
        else if(glutamine < 11*bracket){
            _state = 18;
        }
        else if(glutamine < 12*bracket){
            _state = 17;
        }
        else if(glutamine < 13*bracket){
            _state = 16;
        }
        else if(glutamine <= _energyStorageCap){
            _state = 15;
        }
    }
}

//Change particle color
int EnergyParticle::headMarkColor() const {
   int color = 0x00FF00;
   int color2 = 0x00FFFF;
   int color3 = 0xFF0000;
  switch(_state) {
    case 0:   return 800080;
    case 1:   return interpolate(0xFFFFFF,color,1.0);
    case 2:   return interpolate(0xFFFFFF,color,0.9);
    case 3:   return interpolate(0xFFFFFF,color,0.8);
    case 4:   return interpolate(0xFFFFFF,color,0.7);
    case 5:   return interpolate(0xFFFFFF,color,0.6);
    case 6:   return interpolate(0xFFFFFF,color,0.5);
    case 7:  return interpolate(0xFFFFFF,color,0.4);
    case 8:  return interpolate(0xFFFFFF,color,0.3);
    case 9:  return interpolate(0xFFFFFF,color,0.27);
    case 10:  return interpolate(0xFFFFFF,color,0.25);
    case 11:  return interpolate(0xFFFFFF,color,0.24);
    case 12:  return interpolate(0xFFFFFF,color,0.22);
    case 13:  return interpolate(0xFFFFFF,color,0.2);
    case 14:  return interpolate(0xFFFFFF,color,0.15);

    case 15:   return interpolate(0xFFFFFF,color2,1.0);
    case 16:   return interpolate(0xFFFFFF,color2,0.9);
    case 17:   return interpolate(0xFFFFFF,color2,0.8);
    case 18:   return interpolate(0xFFFFFF,color2,0.7);
    case 19:   return interpolate(0xFFFFFF,color2,0.6);
    case 20:   return interpolate(0xFFFFFF,color2,0.5);
    case 21:  return interpolate(0xFFFFFF,color2,0.4);
    case 22:  return interpolate(0xFFFFFF,color2,0.3);
    case 23:  return interpolate(0xFFFFFF,color2,0.27);
    case 24:  return interpolate(0xFFFFFF,color2,0.25);
    case 25:  return interpolate(0xFFFFFF,color2,0.24);
    case 26:  return interpolate(0xFFFFFF,color2,0.22);
    case 27:  return interpolate(0xFFFFFF,color2,0.2);
    case 28:  return interpolate(0xFFFFFF,color2,0.15);

  case 29:   return interpolate(0xFFFFFF,color3,1.0);
  case 30:   return interpolate(0xFFFFFF,color3,0.9);
  case 31:   return interpolate(0xFFFFFF,color3,0.8);
  case 32:   return interpolate(0xFFFFFF,color3,0.7);
  case 33:   return interpolate(0xFFFFFF,color3,0.6);
  case 34:   return interpolate(0xFFFFFF,color3,0.5);
  case 35:  return interpolate(0xFFFFFF,color3,0.4);
  case 36:  return interpolate(0xFFFFFF,color3,0.3);
  case 37:  return interpolate(0xFFFFFF,color3,0.27);
  case 38:  return interpolate(0xFFFFFF,color3,0.25);
  case 39:  return interpolate(0xFFFFFF,color3,0.24);
  case 40:  return interpolate(0xFFFFFF,color3,0.22);
  case 41:  return interpolate(0xFFFFFF,color3,0.2);
  case 42:  return interpolate(0xFFFFFF,color3,0.15);
  }
  return -1;
}

//Generate color gradient
int EnergyParticle::interpolate(int color1, int color2, double intensity) const {
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

int EnergyParticle::tailMarkColor() const {
  return headMarkColor();
}

void EnergyParticle::setState(int s){
    _state=s;
}

int EnergyParticle::getState(){
    return _state;
}

//Amoebot particle wrapper functions

EnergyParticle& EnergyParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<EnergyParticle>(label);
}

bool EnergyParticle::hasNbrAtLabel(int label) const{
    return AmoebotParticle::hasNbrAtLabel(label);
}

//Initialization helper functions
void EnergyParticle::expandInit(int dir){
    this->expand(dir);
}

void EnergyParticle::contractInit(){
    this->contractHead();
}


//Particle system initialization
EnergyDistributionSystem::EnergyDistributionSystem(double consumptionRate, double restrictedRate,
                                     double hungerThreshold, double energyStorageCap,
                                     double environmentalGlutamate, double glutamateCost, double ammoniumBenefit, double inhibitedReuptakeRate, int signalSpeed){
    //Initialize global list
    allParticles.clear();
    //Initialize default variables
    _consumptionRate = consumptionRate;
    _hungerThreshold = hungerThreshold;
    _energyStorageCap = energyStorageCap;
    _environmentalGlutamate = environmentalGlutamate;
    _glutamateCost = glutamateCost;
    _ammoniumBenefit = ammoniumBenefit;
    _signalSpeed = signalSpeed;
    _restrictedRate=restrictedRate;
    _inhibitedReuptakeRate = inhibitedReuptakeRate;
    //Initialize root particle
    Node node(1,1);
    EnergyParticle* rootNode = new EnergyParticle(node,-1,randDir(),*this,_consumptionRate,_restrictedRate,_hungerThreshold,
                                           _energyStorageCap,_environmentalGlutamate,_glutamateCost,_ammoniumBenefit,_inhibitedReuptakeRate,_signalSpeed);
    rootNode->_treeState= EnergyParticle::root;
    insert(rootNode);
    allParticles.push_back(rootNode);

    //Initialize inner 6 particles
    EnergyParticle** particles = new EnergyParticle*[7];
    int c = 0;
    for(int i = 0; i <3;++i){
        for(int j = 0; j <3;++j){
            if((i==0 && j == 0)||(i==2 && j==2)||(i==1&&j==1)){
                continue;
            }
                    Node node(i,j);
                    EnergyParticle* particle = new EnergyParticle(node,-1,randDir(),*this,_consumptionRate, _restrictedRate,
                                                                  _hungerThreshold,_energyStorageCap,
                                                                  _environmentalGlutamate,_glutamateCost,_ammoniumBenefit,_inhibitedReuptakeRate,_signalSpeed);
                    particles[c]= particle;
                    insert(particle);
                    particle->_parent = rootNode;
                    rootNode->_children[c] = particle;
                    particle->_treeState = EnergyParticle::follower;
                    c++;
                    allParticles.push_back(particle);
        }
    }
    /*
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
    */
/*
    //Place Root on periphery
    allParticles.back()->_parent = nullptr;
    allParticles.back()->_children[0] = root;
    root->_parent = allParticles.back();
    root->_children[6]=nullptr;
*/
    /*
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
    */
//    numTotalParticles = numPrevParticles = 7;
    //this->_numMovements = 0;
    //file.open ("special_data.txt");

}



