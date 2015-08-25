#ifndef COMPACTION_H
#define COMPACTION_H

#include "alg/algorithmwithflags.h"

class System;

namespace Compaction
{

enum class State {
    Seed,
    Idle,
    Active,
    Leader,
    Follower
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

    static std::shared_ptr<System> instance(const unsigned int size);

    virtual Movement execute();

    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone() override;

    virtual bool isDeterministic() const;
    virtual bool isStatic() const;

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
