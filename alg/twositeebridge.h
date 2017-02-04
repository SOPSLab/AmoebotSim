#ifndef TWOSITEEBRIDGE_H
#define TWOSITEEBRIDGE_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class TwoSiteEBridgeParticle : public AmoebotParticle
{
    friend class TwoSiteEBridgeSystem;

public:
    enum class Role {
        Particle, Site, All
    };

    TwoSiteEBridgeParticle(const Node head,
                           const int globalTailDir,
                           const int orientation,
                           AmoebotSystem& system,
                           Role role,
                           const float explambda,
                           const float complambda,
                           const float alpha);

    virtual void activate();
    virtual int headMarkColor() const;
    virtual QString inspectionText() const;

    TwoSiteEBridgeParticle& neighborAtLabel(int label) const;

protected:
    const Role role;
    const float explambda, complambda, alpha;
    float lambda, q;
    int numParticleNbrs1, numSiteNbrs1;
    bool flag;

private:
    bool hasExpandedNeighbor() const;
    int neighborCount(std::vector<int> labels, const Role r) const;
    bool checkProp1(const Role r) const;
    bool checkProp2(const Role r) const;
    const std::vector<int> uniqueLabels() const;
    const std::vector<int> occupiedLabelsNoExpandedHeads(std::vector<int> labels, const Role r) const;
};

class TwoSiteEBridgeSystem : public AmoebotSystem
{
public:
    TwoSiteEBridgeSystem(int numParticles = 100, float explambda = 2.0, float complambda = 4.0, float alpha = 1.0);

    virtual bool hasTerminated() const;
};

#endif // TWOSITEEBRIDGE_H

