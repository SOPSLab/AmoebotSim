#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "alg/algorithmwithflags.h"

class System;

namespace Triangle
{
enum class State {
    Finished,
    Leader,
    Follower,
    Idle,
    Seed
};

class TriFlag : public Flag
{
public:
    TriFlag();
    TriFlag(const TriFlag& other);
    Triangle::State state;
    bool point;
    bool side;
    bool followIndicator;
    int contractDir;
};

class Triangle : public AlgorithmWithFlags<TriFlag>
{
public:
    Triangle(const State _state);
    Triangle(const Triangle& other);
    virtual ~Triangle();

    static System* instance(const int size, const double holeProb);

    virtual Movement execute();
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    int isPointedAt();

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
};
}

#endif // TRIANGLE_H
