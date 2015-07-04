#ifndef HOLEELIMINATION_H
#define HOLEELIMINATION_H

#include "alg/algorithmwithflags.h"

class System;

namespace HoleElimination
{

enum class State {
    Seed,
    Idle,
    Active,
    Leader,
    Follower,
    Finished
};

class HoleEliminationFlag : public Flag
{
public:
    HoleEliminationFlag();
    HoleEliminationFlag(const HoleEliminationFlag& other);

    State state;
    bool isParent;
    int oldFollowDir;
    bool dockingIndicator;
};

class HoleElimination : public AlgorithmWithFlags<HoleEliminationFlag>
{
public:
    HoleElimination(const State _state);
    HoleElimination(const HoleElimination& other);
    virtual ~HoleElimination();

    static System* instance(const unsigned int size, const double holeProb);

    virtual Movement execute();

    virtual std::shared_ptr<Algorithm> clone() override;

    virtual bool isDeterministic() const;

protected:
    void setState(const State _state);
    void setOldFollowDir(const int dir);
    void setParentLabel(const int parentLabel);
    void unsetParentLabel();
    void setDockingLabel(const int dockingLabel);
    void unsetDockingLabel();

    bool hasNeighborInState(const State _state) const;
    int neighborInStateDir(const State _state) const;
    int emptyNeighborDir() const;
    int leafSwitchDir() const;
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

#endif // HOLEELIMINATION_H
