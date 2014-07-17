#ifndef RHOMBOID_H
#define RHOMBOID_H

#include "alg/algorithmwithflags.h"

class System;

namespace Rhomboid
{
enum class State {
    Finished,
    Leader,
    Follower,
    Idle,
    Seed
};

class RhomFlag : public Flag
{
public:
    RhomFlag();
    RhomFlag(const RhomFlag& other);
    State state;
    bool point;
    int count;
    bool followIndicator;
    int contractDir;
};

class Rhomboid : public AlgorithmWithFlags<RhomFlag>
{
public:
    Rhomboid(const State _state, const int _sideLength);
    Rhomboid(const Rhomboid& other);
    virtual ~Rhomboid();

    static System* instance(const unsigned int size, const double holeProb, const int sideLength);

    virtual Movement execute();
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
	int isPointedAt();
    int getCount();
    void setCount(const int _count);

    void setState(State _state);
    bool neighborInState(int direction, State _state);
    bool hasNeighborInState(State _state);
    int firstNeighborInState(State _state);

    int getMoveDir();
    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator();
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;
    bool followIndicatorMatchState(State _state) const;

protected:
    State state;
    int followDir;
    int sideLength;
};
}

#endif // RHOMBOID_H
