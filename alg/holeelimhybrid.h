#ifndef HOLEELIMHYBRID_H
#define HOLEELIMHYBRID_H

#include "alg/algorithmwithflags.h"

class System;

namespace HoleElimHybrid
{

enum class State {
    Seed,
    Idle,
    Active,
    Leader,
    Follower,
    Walking,
    Finished
};

class HoleElimHybridFlag : public Flag
{
public:
    HoleElimHybridFlag();
    HoleElimHybridFlag(const HoleElimHybridFlag& other);

    State state;
    bool isParent;
    int oldFollowDir;
    bool dockingIndicator;
};

class HoleElimHybrid : public AlgorithmWithFlags<HoleElimHybridFlag>
{
public:
    HoleElimHybrid(const State _state);
    HoleElimHybrid(const HoleElimHybrid& other);
    virtual ~HoleElimHybrid();

    static std::shared_ptr<System> instance(const unsigned int size, const double holeProb);

    virtual Movement execute();

    virtual std::shared_ptr<Algorithm> clone() override;

    virtual bool isDeterministic() const;

protected:
    void setState(const State _state);
    void setOldFollowDir(const int dir);
    void setParentLabel(const int parentLabel);
    void unsetParentLabel();
    void setDockingLabel(const int dockingLabel);

    bool hasNeighborInState(const State _state) const;
    int neighborInStateDir(const State _state) const;
    int emptyNeighborDir() const;
    int dockingDir() const;
    int adjFinishDir() const;
    int borderFinishedDir() const;

    bool isLocallyCompact() const;
    bool isParent() const;
    bool isLeaf() const;
    bool tailHasChild() const;
    bool leaderAsChild() const;

    State state;
    int followDir;

private:
    int countNeighbors() const;
};

}

#endif // HOLEELIMHYBRID_H
