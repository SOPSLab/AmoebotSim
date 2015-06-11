#ifndef COMPACT2_H
#define COMPACT2_H

#include "alg/algorithmwithflags.h"

class System;

namespace Compact2
{

enum class State {
    Seed,
    Leader,
    Follower,
    Active,
    Idle
};

class Compact2Flag : public Flag
{
public:
    Compact2Flag();
    Compact2Flag(const Compact2Flag& other);

    State state;
    bool isParent;
    int oldFollowDir;
};

class Compact2 : public AlgorithmWithFlags<Compact2Flag>
{
public:
    Compact2(const State _state);
    Compact2(const Compact2& other);
    virtual ~Compact2();

    static System* instance(const unsigned int size, const double holeProb);

    virtual Movement execute();

    virtual Algorithm* clone();

    virtual bool isDeterministic() const;

protected:
    void setState(State _state);
    void setOldFollowDir(int dir);
    void setParentLabel(int parentLabel);
    void unsetParentLabel();

    bool hasNeighborInState(State _state);
    int neighborInStateDir(State _state);
    int emptyNeighborDir();
    int determineFollowDir();

    bool isLocallyCompact();
    bool isParent();
    bool tailHasChild();
    bool leaderAsChild();

protected:
    State state;
    int followDir;
};

}

#endif // COMPACT2_H
