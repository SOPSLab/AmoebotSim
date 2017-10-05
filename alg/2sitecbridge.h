#ifndef TWOSITECBRIDGE_H
#define TWOSITECBRIDGE_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class TwoSiteCBridgeParticle : public AmoebotParticle
{
    friend class TwoSiteCBridgeSystem;

public:
    enum class Role {
        Particle,
        Site,
        All
    };

    TwoSiteCBridgeParticle(const Node head,
                        const int globalTailDir,
                        const int orientation,
                        AmoebotSystem& system,
                        Role role,
                        const float lambda,
                        const float alpha);

    virtual void activate();
    virtual int headMarkColor() const;
    virtual QString inspectionText() const;

    TwoSiteCBridgeParticle& nbrAtLabel(int label) const;

protected:
    const Role role;
    const float lambda, alpha;
    float q;
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

class TwoSiteCBridgeSystem : public AmoebotSystem
{
public:
    TwoSiteCBridgeSystem(int numParticles = 200, float lambda = 4.0, float alpha = 1.0);

    virtual bool hasTerminated() const;
};


#endif // TWOSITECBRIDGE_H

