#ifndef INFOBJCOATING_H
#define INFOBJCOATING_H

#include "alg/algorithm.h"

class InfObjCoatingFlag;
class System;

class InfObjCoating : public Algorithm
{
public:
    enum class Phase {
        Inactive,
        Follow,
        Lead
    };

public:
    InfObjCoating(const Phase _phase);
    InfObjCoating(const InfObjCoating& other);
    virtual ~InfObjCoating();

    static System* instance(const int size);

    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);

    void setPhase(const Phase _phase);
    int neighborInPhase(const Phase _phase) const;
    bool hasNeighborInPhase(const Phase _phase) const;

    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void setFollowIndicatorLabel(const int label);
    bool receivesFollowIndicator() const;

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
