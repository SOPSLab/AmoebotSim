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
                           const double explambda,
                           const double complambda,
                           const bool flag);

    virtual void activate();
    virtual int headMarkColor() const;
    virtual int tailMarkColor() const;
    virtual QString inspectionText() const;

    TwoSiteEBridgeParticle& nbrAtLabel(int label) const;

protected:
    const Role role;
    const double explambda, complambda;
    double lambda, q;
    int numNbrs1;
    bool flag;

private:
    bool hasExpandedNeighbor() const;
    bool hasNeighborWithLambda(const int val) const;
    int neighborCount(std::vector<int> labels, const Role r, const bool countNewSites) const;
    bool checkProp1(const Role r) const;
    bool checkProp2(const Role r) const;
    const std::vector<int> uniqueLabels() const;
    const std::vector<int> occupiedLabelsNoExpandedHeads(std::vector<int> labels, const Role r, const bool countNewSites) const;
};

class TwoSiteEBridgeSystem : public AmoebotSystem
{
public:
    TwoSiteEBridgeSystem(int numParticles = 100, double explambda = 2.0, double complambda = 4.0, double siteDistance = 1.25);

    virtual bool hasTerminated() const;
};

#endif // TWOSITEEBRIDGE_H

