#ifndef COMPACTION_H
#define COMPACTION_H

#include "alg/algorithmwithflags.h"

class System;

namespace Compaction
{

enum class State {
    Seed,
    Leader,
    Follower,
    Active,
    Idle
};

class CompactionFlag : public Flag
{
public:
    CompactionFlag();
    CompactionFlag(const CompactionFlag& other);

    State state;
    bool isParent;
    int oldFollowDir;
};

class Compaction : public AlgorithmWithFlags<CompactionFlag>
{
public:
    Compaction(const State _state);
    Compaction(const Compaction& other);
    virtual ~Compaction();

    static System* instance(const unsigned int size, const double holeProb);

    virtual Movement execute();

    virtual Algorithm* clone();

    virtual bool isDeterministic() const;

protected:
    void setState(const State _state);
    void setOldFollowDir(const int dir);
    void setParentLabel(const int parentLabel);
    void unsetParentLabel();

    bool hasNeighborInState(const State _state) const;
    int neighborInStateDir(const State _state) const;
    int emptyNeighborDir() const;
    int leafSwitchDir() const;

    bool isLocallyCompact() const;
    bool isParent() const;
    bool isLeaf() const;
    bool tailHasChild() const;
    bool leaderAsChild() const;

protected:
    State state;
    int followDir;

private:
    int countNeighbors() const;
};

}

#endif // COMPACTION_H
