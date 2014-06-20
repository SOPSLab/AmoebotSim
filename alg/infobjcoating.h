#ifndef INFOBJCOATING_H
#define INFOBJCOATING_H

#include "alg/algorithm.h"

class InfObjCoatingFlag;
class System;

class InfObjCoating : public Algorithm
{
public:
    enum class Phase {
        Static,
        Inactive,
        Follow,
        Lead
    };

public:
    InfObjCoating(const Phase _phase);
    InfObjCoating(const InfObjCoating& other);
    virtual ~InfObjCoating();

    static System* instance(const int numParticles, const float holeProb);

    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);

    void setPhase(const Phase _phase);

    bool neighborIsInPhase(const int label, const Phase _phase) const;
    int firstNeighborInPhase(const Phase _phase) const;
    bool hasNeighborInPhase(const Phase _phase) const;

    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator() const;
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;

    int getMoveDir() const;

protected:
    std::array<const InfObjCoatingFlag*, 10> inFlags;
    std::array<InfObjCoatingFlag*, 10> outFlags;

    Phase phase;
    int followDir;
};

class InfObjCoatingFlag : public Flag
{
public:
    InfObjCoatingFlag();
    InfObjCoatingFlag(const InfObjCoatingFlag& other);

public:
    InfObjCoating::Phase phase;
    int contractDir;
    bool followIndicator;
};

#endif // INFOBJCOATING_H
