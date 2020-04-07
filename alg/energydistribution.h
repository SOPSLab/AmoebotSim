// Copyright (C) 2018 Robert Gmyr, Joshua J. Daymude, and Kristian Hinnenthal
// The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
// notice can be found at the top of main/main.cpp.
//
// Defines the particle system and composing particles for the Disco code
// tutorial, a first algorithm for new developers to AmoebotSim. Disco
// demonstrates the basics of algorithm architecture, instantiating a particle
// system, moving particles, and changing particles' states. The pseudocode is
// available in the wiki:
// [https://bitbucket.org/gmyr/amoebotsim/wiki/Code%20Tutorials].

#ifndef HAMILTONIANSYSTEM_H
#define HAMILTONIANSYSTEM_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class EnergyParticle : public AmoebotParticle {
    public:
    EnergyParticle(const Node head, const int globalTailDir,const int orientation, AmoebotSystem& system,
                   double consumptionRate, double restrictedRate,
                   double hungerThreshold, double energyStorageCap,
                   double environmentalGlutamate, double glutamateCost, double ammoniumBenefit, double inhibitedReuptakeRate, int signalSpeed);
    //Methods
    bool hasNbrAtLabel(int) const;
    EnergyParticle& mitosis(int,int);
    EnergyParticle& nbrAtLabel(int) const;
    EnergyParticle* argMax(int);
    bool hasEmptyNeighbor();
    int headMarkColor() const override;
    int tailMarkColor() const override;
    int getState();
    int interpolate(int,int,double) const;
    void activate() override;
    void updateState();
    void setState(int);
    void expandInit(int);
    void contractInit();
    void globalMetrics();
    void globalInteriorEnergy();
    void communicate();
    void harvestEnergy();
    void harvestRegulant();
    void produceRegulant();
    void reproduce();
    int hasEmptyNeighborInDir();

    //local variables
    AmoebotSystem* _sys;
    double _energyBattery;
    double _energyBuffer;
    double _regulantBattery;
    double _regulantBuffer;
    bool _stress;
    bool _inhibit;


    bool _inTelophase;
    bool _stressed;
    bool _stressRoot;
    bool _canEat;
    double _consumedGlutamate;
    double _consumedAmmonium;
    double _remainingGlutamate;
    double _remainingAmmonium;
    double _effectiveRate;
    EnergyParticle* _parent;
    EnergyParticle** _children;
    int _expandDir;
    int _prevX;
    int _prevY;
    int _signalTimer;
    int _phase;


    //inherited local variables
    double _consumptionRate;
    double _restrictedRate;
    double _hungerThreshold;
    double _energyStorageCap;
    double _environmentalGlutamate;
    double _glutamateCost;
    double _ammoniumBenefit;
    double _inhibitedReuptakeRate;
    double _GDH;
    double _regulantConversion;
    int _signalSpeed;

    protected:
        int _state;
        enum TreeState {idle, follower, root}_treeState;

     private:
        friend class EnergyDistributionSystem;

};

class EnergyDistributionSystem : public AmoebotSystem {
    public:
        EnergyDistributionSystem(double consumptionRate, double restrictedRate,
                          double hungerThreshold, double energyStorageCap, double environmentalGlutamate,
                          double glutamateCost, double ammoniumBenefit, double inhibitedReuptakeRate, int signalSpeed);
    private:
        double _consumptionRate;
        double _restrictedRate;
        double _hungerThreshold;
        double _energyStorageCap;
        double _environmentalGlutamate;
        double _glutamateCost;
        double _ammoniumBenefit;
        double _inhibitedReuptakeRate;
        int _signalSpeed;
};

#endif // HAMILTONIANSYSTEM_H

