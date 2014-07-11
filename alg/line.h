#ifndef LINE_H
#define LINE_H

#include "alg/algorithmwithflags.h"

class System;

namespace Line
{
enum class State {
    Finished,
    Leader,
    Follower,
    Idle,
    Seed
};

class LineFlag : public Flag
{
public:
    LineFlag();
    LineFlag(const LineFlag& other);
    State state;
    bool point;
    bool followIndicator;
    int contractDir;
};

class Line : public AlgorithmWithFlags<LineFlag>
{
public:
    Line(const State _state);
    Line(const Line& other);
    virtual ~Line();

    static System* instance(const unsigned int size, const double holeProb);

    virtual Movement execute();
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    int isPointedAt(); //

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

#endif // LINE_H

