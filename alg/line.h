// Brian Parker
// Note that all of my shape formation algorithms make use of "State" instead of "phase". The two are equivalent in theory, but not in programming.
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
    bool point; //
    bool followIndicator;
    int contractDir;
};

class Line : public AlgorithmWithFlags<LineFlag>
{
public:
    Line(const State _state);
    Line(const Line& other);
    virtual ~Line();

    static std::shared_ptr<System> instance(const unsigned int size, const double holeProb);

    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> clone() override;
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

