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
        Object
    };

    TwoSiteCBridgeParticle(const Node head,
                        const int globalTailDir,
                        const int orientation,
                        AmoebotSystem& system,
                        Role role,
                        const float lambda);

    virtual void activate();
    virtual int headMarkColor() const;
    virtual QString inspectionText() const;

    TwoSiteCBridgeParticle& neighborAtLabel(int label) const;

protected:
    const Role role;
    const float lambda;
    float q;
    int numNbrsBefore;
    bool flag;

private:
    bool hasExpandedNeighbor() const;
    int neighborCount(std::vector<int> labels) const;
    bool checkProp1(const int sizeS) const;
    bool checkProp2(const int sizeS) const;
    const std::vector<int> uniqueLabels() const;
    const std::vector<int> occupiedLabelsNoExpandedHeads(std::vector<int> labels) const;
};

class TwoSiteCBridgeSystem : public AmoebotSystem
{
public:
    TwoSiteCBridgeSystem(int numParticles = 200, float lambda = 4.0);

    virtual bool hasTerminated() const;
};


#endif // TWOSITECBRIDGE_H

