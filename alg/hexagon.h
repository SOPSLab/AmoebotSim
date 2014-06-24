#ifndef HEXAGON_H
#define HEXAGON_H

#include "alg/algorithmwithflags.h"

class System;

namespace Hexagon
{
enum class State {
    Finished,
    Leader,
    Follower,
    Idle,
    Seed
};

class HexFlag : public Flag
{
public:
    HexFlag();
    HexFlag(const HexFlag& other);
    State state;
    bool point;
    bool followIndicator;
    int contractDir;
};

class Hexagon : public AlgorithmWithFlags<HexFlag>
{
public:
    Hexagon(const State _state);
    Hexagon(const Hexagon& other);
    virtual ~Hexagon();

    static System* instance(const int size, const double holeProb);

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

#endif // HEXAGON_H

