#ifndef BOUNDEDOBJCOATING_H
#define BOUNDEDOBJCOATING_H

#include "alg/algorithmwithflags.h"

class System;

namespace BoundedObjCoating
{
enum class Phase {
    Static,
    Border,
    Inactive,
    retiredLeader,
    Follow,
    Lead
};

class BoundedObjCoatingFlag : public Flag
{
public:
    BoundedObjCoatingFlag();
    BoundedObjCoatingFlag(const BoundedObjCoatingFlag& other);

public:
    Phase phase;
    int contractDir;
    bool followIndicator;
    int Lnumber;
    int NumFinishedNeighbors;
};

class BoundedObjCoating : public AlgorithmWithFlags<BoundedObjCoatingFlag>
{
public:
    BoundedObjCoating(const Phase _phase);
    BoundedObjCoating(const BoundedObjCoating& other);
    virtual ~BoundedObjCoating();

    static std::shared_ptr<System> instance(const int numStaticParticles, const int numParticles, const float holeProb);

    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> blank() override;
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;

protected:
    void setPhase(const Phase _phase);

    bool neighborIsInPhase(const int label, const Phase _phase) const;
    int firstNeighborInPhase(const Phase _phase) const;
    bool hasNeighborInPhase(const Phase _phase) const;

    int countGreenNeighbors(const Phase _phase, const int L) const;
    void setLayerNumber(const int _Lnumber);
    void setNumFinishedNeighbors(const int _NumFinishedNeighbors);
    int CountFinishedSides(const int _leftDir, const int _rightDir) const;
    int countRetiredareFinished(const int _Lnumber) const;
    void getLeftDir() ;

    int firstNeighborInPhaseandLayer(const Phase _phase, const int L) const;
    bool neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const;


    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator();
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;

    int getMoveDir() const;
    int getDownDir() const;
    bool getLnumber1() const;
    int getLnumber() const;

protected:
    Phase phase;
    int followDir;
    int Lnumber;
    int downDir;
    int leftDir;
    int rightDir;
    int NumFinishedNeighbors;
};
}

#endif // BOUNDEDOBJCOATING_H
